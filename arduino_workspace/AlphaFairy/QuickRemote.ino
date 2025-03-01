#include "AlphaFairy.h"

#define QIKRMT_ROLL_SPAN 60
#define QIKRMT_HYSTER    3
#define QIKRMT_FPULL_Y   198
enum
{
    QIKRMTBTN_IDLE,
    QIKRMTBTN_PRESSED_LOCKING_WAIT,
    QIKRMTBTN_PRESSED_UNLOCKING_WAIT,
};

enum
{
    QIKRMTIMU_FREE,
    QIKRMTIMU_FREE_TEMP,
    QIKRMTIMU_LOCKED,
};

uint8_t qikrmt_imuState = QIKRMTIMU_LOCKED;
uint8_t qikrmt_col = 0;
uint8_t qikrmt_row = 0;
int8_t  qikrmt_col_prev = -1;
int8_t  qikrmt_row_prev = -1;
int8_t  qikrmt_imustate_prev = -1;
int     qikrmt_roll_center = 0;

void qikrmt_task(bool freeze_row)
{
    int ang;

    uint32_t now = millis();
    static uint32_t btn_down_time = 0;

    // lock or unlock the IMU selection, via side button
    if (btnSide_hasPressed())
    {
        btn_down_time = millis();
        btnSide_clrPressed();
        if (qikrmt_imuState == QIKRMTIMU_LOCKED) {
            qikrmt_imuState = QIKRMTIMU_FREE;
        }
        else {
            qikrmt_imuState = QIKRMTIMU_LOCKED;
        }
    }
    // holding down the side button means the unlock is only temporary
    else if (btnSide_isPressed() && btn_down_time > 0 && (qikrmt_imuState == QIKRMTIMU_FREE_TEMP || qikrmt_imuState == QIKRMTIMU_FREE))
    {
        if ((now - btn_down_time) > 1000)
        {
            qikrmt_imuState = QIKRMTIMU_FREE_TEMP;
        }
    }
    else if (btnSide_isPressed() == false)
    {
        // end the temporary unlock if needed
        if (qikrmt_imuState == QIKRMTIMU_FREE_TEMP) {
            qikrmt_imuState = QIKRMTIMU_LOCKED;
        }
    }

    if (qikrmt_imuState == QIKRMTIMU_FREE || qikrmt_imuState == QIKRMTIMU_FREE_TEMP)
    {
        if (freeze_row == false)
        {
            ang = imu.rolli;
            ang = ang > 90 ? 90 : (ang < -90 ?  -90 : ang); // limit

            if (ang > qikrmt_roll_center + (QIKRMT_ROLL_SPAN / 2)) { // exceeded boundary, shift the center point
                qikrmt_roll_center = ang - (QIKRMT_ROLL_SPAN / 2);
                qikrmt_row = 2;
            }
            else if (ang < qikrmt_roll_center - (QIKRMT_ROLL_SPAN / 2)) { // exceeded boundary, shift the center point
                qikrmt_roll_center = ang + (QIKRMT_ROLL_SPAN / 2);
                qikrmt_row = 0;
            }
            // pick a row based on the angle, centered over a particular angle representing the center
            else if (qikrmt_row == 0 && ang >= qikrmt_roll_center - (QIKRMT_ROLL_SPAN / (3 * 2)) + QIKRMT_HYSTER)
            {
                qikrmt_row = 1;
            }
            else if (qikrmt_row == 1 && ang >= qikrmt_roll_center + (QIKRMT_ROLL_SPAN / (3 * 2)) + QIKRMT_HYSTER)
            {
                qikrmt_row = 2;
            }
            else if (qikrmt_row == 1 && ang <= qikrmt_roll_center - (QIKRMT_ROLL_SPAN / (3 * 2)) - QIKRMT_HYSTER)
            {
                qikrmt_row = 0;
            }
            else if (qikrmt_row == 2 && ang <= qikrmt_roll_center + (QIKRMT_ROLL_SPAN / (3 * 2)) - QIKRMT_HYSTER)
            {
                qikrmt_row = 1;
            }
        }

        ang = imu.pitchi;
        ang = ang > 90 ? 90 : (ang < -90 ?  -90 : ang); // limit

        // pick the column based on angle, since this is the pitch angle, always use 0 as center
        if (qikrmt_col == 0 && ang >= QIKRMT_HYSTER) {
            qikrmt_col = 1;
        }
        else if (qikrmt_col == 1 && ang <= -QIKRMT_HYSTER) {
            qikrmt_col = 0;
        }
    }

    // draw the box if needed
    if (qikrmt_col != qikrmt_col_prev || qikrmt_row != qikrmt_row_prev || qikrmt_imuState != qikrmt_imustate_prev || redraw_flag)
    {
        cpufreq_boost();
        pwr_tick(true); // movement means don't turn off

        // first draw a white box over the previous coordinate to remove the box
        if (qikrmt_row_prev >= 0 && qikrmt_col_prev >= 0) {
            qikrmt_drawBox(qikrmt_row_prev, qikrmt_col_prev, TFT_WHITE);
        }

        // draw the new box
        qikrmt_drawBox(qikrmt_row, qikrmt_col, qikrmt_imuState == QIKRMTIMU_LOCKED ? TFT_BLACK : TFT_ORANGE);

        // blank out the arrows for focus pull when not in focus mode
        if (qikrmt_row != 2) {
            M5Lcd.fillRect(0, QIKRMT_FPULL_Y, M5Lcd.width(), 26, TFT_WHITE);
        }
    }

    // when in focus pull mode, indicate step size of focus change
    if (qikrmt_row == 2) {
        gui_drawFocusPullState(QIKRMT_FPULL_Y);
    }

    qikrmt_col_prev = qikrmt_col;
    qikrmt_row_prev = qikrmt_row;
    qikrmt_imustate_prev = qikrmt_imuState;
}

void qikrmt_drawBox(uint8_t row, uint8_t col, uint16_t colour)
{
    uint16_t linewidth = M5Lcd.width();
    uint16_t xstart = 1, xwidth = linewidth / 2;
    if (col == 1 && row != 2) {
        xstart = (linewidth / 2) + 1;
    }
    uint16_t boxheight = 50;
    uint16_t ystart = 44 + (boxheight * row);
    M5Lcd.drawRect(xstart    , ystart    , (row == 2 ? linewidth : (linewidth / 2)) - 2, boxheight    , colour);
    M5Lcd.drawRect(xstart + 1, ystart + 1, (row == 2 ? linewidth : (linewidth / 2)) - 4, boxheight - 2, colour); // thicker
}

#include "FairyMenu.h"

class AppQuickRemote : public FairyMenuItem
{
    public:
        AppQuickRemote() : FairyMenuItem("/qikrmt_faded.png") // main image is the faded version, the loop will draw the active version when required
        {
            reset();
        };

        virtual void reset(void)
        {
            qikrmt_imuState = QIKRMTIMU_LOCKED;
            qikrmt_col = 0;
            qikrmt_row = 0;
            qikrmt_col_prev = -1;
            qikrmt_row_prev = -1;
            qikrmt_imustate_prev = -1;
            qikrmt_roll_center = 0;
        };

        virtual bool on_execute(void)
        {
            reset();
            set_redraw();
            app_waitAllRelease();

            while (true)
            {
                app_poll();

                if (redraw_flag) {
                    M5Lcd.drawPngFile(SPIFFS, "/qikrmt_active.png", 0, 0);
                }

                qikrmt_task(false);       // draw the table, with the indicator position set by IMU polling
                gui_drawStatusBar(false);
                pwr_sleepCheck();

                redraw_flag = false;

                if (btnBig_hasPressed())
                {
                    btnBig_clrPressed();

                    bool can_do = true;

                    if (qikrmt_row == 0 && qikrmt_col == 0) { // remote shutter
                        remote_shutter(0, false);
                    }
                    else if (qikrmt_row == 0 && qikrmt_col == 1) { // record movie
                        record_movie();
                    }
                    else if (qikrmt_row == 1) // zoom
                    {
                        if (must_be_connected() == false) {
                            can_do = false;
                        }

                        if (can_do)
                        {
                            bool do_one = true;
                            while ((btnBig_isPressed() || do_one) && fairycam.isOperating())
                            {
                                do_one = false;
                                cpufreq_boost();
                                app_poll();
                                int8_t dir = qikrmt_col == 0 ? -1 : +1; // pick direction of zoom based on which table column is selected
                                if (ptpcam.isOperating())
                                {
                                    if (dir != 0) {
                                        ptpcam.cmd_ZoomStep((dir > 0) ? -1 : ((dir < 0) ? +1 : 0)); // I am soooo sorry for this
                                        ptpcam.wait_while_busy(config_settings.focus_pause_time_ms, DEFAULT_BUSY_TIMEOUT, NULL);
                                    }
                                }
                                if (httpcam.isOperating())
                                {
                                    httpcam.cmd_ZoomStart(dir);
                                    httpcam.wait_while_busy(config_settings.focus_pause_time_ms, DEFAULT_BUSY_TIMEOUT, NULL);
                                }
                            }

                            // button is released, stop the zooming
                            if (ptpcam.isOperating()) {
                                ptpcam.cmd_ZoomStep(0);
                            }
                            if (httpcam.isOperating()) {
                                httpcam.cmd_ZoomStop();
                            }
                        }
                    }
                    else if (qikrmt_row == 2) // focus
                    {
                        if (must_be_ptp() == false) {
                            can_do = false;
                        }

                        if (can_do) {
                            focus_pull(true, QIKRMT_FPULL_Y);
                        }
                    }
                }

                if (btnPwr_hasPressed())
                {
                    // this will quit out of the quick remote mode
                    btnPwr_clrPressed();
                    break;
                }
            } // end of while loop

            draw_mainImage(); // draws the faded image
            return false;
        };
};

extern FairySubmenu menu_remote;
void setup_qikrmt()
{
    static AppQuickRemote app;
    menu_remote.install(&app);
}
