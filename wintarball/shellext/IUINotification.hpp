#ifndef I_UI_NOTIFICATION_HPP
#define I_UI_NOTIFICATION_HPP


// synchronous UI notifications

enum UIResult {
    UI_YES,
    UI_NO,
    UI_CANCEL,
};

enum UIIcon {
    UI_ERROR,
    UI_WARNING,
};

class IUINotification
{
public:
    virtual UIResult YesNoCancel(const char* message, UIIcon icon) = 0;
    virtual UIResult YesNo(const char* message, UIIcon icon) = 0;
    virtual void SetMessage(const char* message) = 0;
    virtual void SetProgress(int progress) = 0;  // from 0 to 255

    virtual bool ShouldCancel() = 0;
};


#endif
