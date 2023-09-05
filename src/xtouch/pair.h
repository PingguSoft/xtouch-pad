#ifndef _XLCD_PAIR
#define _XLCD_PAIR

void xtouch_pair_start()
{
    xtouch_ssdp_loop_exit = false;
    loadScreen(5);
    while (!xtouch_ssdp_loop_exit)
    {
        xtouch_ssdp_loop();
        lv_timer_handler();
        lv_task_handler();
    }

    String storedCode = xtouch_ssdp_getStoredCode(xTouchConfig.xTouchSerialNumber);

    if (storedCode.length() > 0)
    {
        strcpy(xTouchConfig.xTouchAccessCode, storedCode.c_str());
    }
    else
    {
        xtouch_ssdp_loop_exit = false;
        loadScreen(6);
        while (!xtouch_ssdp_loop_exit)
        {
            lv_timer_handler();
            lv_task_handler();
        }
    }
}

#endif