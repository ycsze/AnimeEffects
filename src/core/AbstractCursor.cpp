#include "core/AbstractCursor.h"
#include "XC.h"

namespace core
{

AbstractCursor::AbstractCursor()
    : mEventType(Event_TERM)
    , mEventButton(Button_TERM)
    , mIsPressing()
    , mIsDouble()
    , mScreenPoint()
    , mScreenPos()
    , mScreenVel()
    , mWorldPos()
    , mWorldVel()
    , mSuspendedCount(0)
    , mBlankAfterSuspending()
{
    for (int i = 0; i < Button_TERM; ++i)
    {
        mIsPressing[i] = false;
        mIsDouble[i] = false;
        mBlankAfterSuspending[i] = false;
    }
}


bool AbstractCursor::setMousePress(QMouseEvent* aEvent, const CameraInfo& aCameraInfo)
{
    //qDebug() << "press" << aEvent->button();
    mEventType = Event_Press;

    switch (aEvent->button())
    {
    case Qt::LeftButton:  mEventButton = Button_Left;   break;
    case Qt::MidButton:   mEventButton = Button_Middle; break;
    case Qt::RightButton: mEventButton = Button_Right;  break;
    default:              mEventButton = Button_TERM;   break;
    }

    if (mEventButton != Button_TERM) // fail safe code
    {
        mIsPressing[mEventButton] = true;
    }

    mScreenPoint = aEvent->pos();
    mScreenPos = QVector2D(mScreenPoint);
    mScreenVel = QVector2D(0.0f, 0.0f);
    mWorldPos = aCameraInfo.toWorldPos(mScreenPos);
    mWorldVel = QVector2D(0.0f, 0.0f);

    return mSuspendedCount == 0;
}

bool AbstractCursor::setMouseMove(QMouseEvent* aEvent, const CameraInfo& aCameraInfo)
{
    QVector2D prevScreenPos = mScreenPos;
    QVector2D prevWorldPos = mWorldPos;

    mEventType = Event_Move;
    mEventButton = Button_TERM;

    mScreenPoint = aEvent->pos();
    mScreenPos = QVector2D(mScreenPoint);
    mScreenVel = mScreenPos - prevScreenPos;
    mWorldPos = aCameraInfo.toWorldPos(mScreenPos);
    mWorldVel = mWorldPos - prevWorldPos;

    return mSuspendedCount == 0;
}

bool AbstractCursor::setMouseRelease(QMouseEvent* aEvent, const CameraInfo& aCameraInfo)
{
    //qDebug() << "release" << aEvent->button();
    QVector2D prevScreenPos = mScreenPos;
    QVector2D prevWorldPos = mWorldPos;

    mEventType = Event_Release;

    switch (aEvent->button())
    {
    case Qt::LeftButton:  mEventButton = Button_Left;   break;
    case Qt::MidButton:   mEventButton = Button_Middle; break;
    case Qt::RightButton: mEventButton = Button_Right;  break;
    default:              mEventButton = Button_TERM;   break;
    }

    if (mEventButton != Button_TERM) // fail safe code
    {
        mIsPressing[mEventButton] = false;
        mIsDouble[mEventButton] = false;
    }

    mScreenPoint = aEvent->pos();
    mScreenPos = QVector2D(mScreenPoint);
    mScreenVel = mScreenPos - prevScreenPos;
    mWorldPos = aCameraInfo.toWorldPos(mScreenPos);
    mWorldVel = mWorldPos - prevWorldPos;

    bool shouldUpdate = true;
    if (mSuspendedCount > 0)
    {
        shouldUpdate = false;
    }
    else if (mEventButton != Button_TERM && mBlankAfterSuspending.at(mEventButton))
    {
        mBlankAfterSuspending.at(mEventButton) = false;
        shouldUpdate = false;
    }

    if (!shouldUpdate)
    {
        mEventType = Event_TERM;
        mEventButton = Button_TERM;
    }

    return shouldUpdate;
}

bool AbstractCursor::setMouseDoubleClick(QMouseEvent* aEvent, const CameraInfo& aCameraInfo)
{
    setMousePress(aEvent, aCameraInfo);

    if (mEventButton != Button_TERM) // fail safe code
    {
        mIsDouble[mEventButton] = true;
    }

    return mSuspendedCount == 0;
}

void AbstractCursor::suspendEvent(const std::function<void()>& aEventReflector)
{
    XC_ASSERT(mSuspendedCount >= 0);
    ++mSuspendedCount;
    if (mSuspendedCount == 1)
    {
        for (int i = 0; i < Button_TERM; ++i)
        {
            if (mIsPressing[i]) // invoke a release event
            {
                mEventType = Event_Release;
                mEventButton = (Button)i;

                mIsPressing[i] = false;
                mScreenVel = QVector2D();
                mWorldVel = QVector2D();

                aEventReflector();
            }
        }
    }
}

void AbstractCursor::resumeEvent()
{
    XC_ASSERT(mSuspendedCount > 0);
    --mSuspendedCount;

    if (mSuspendedCount == 0)
    {
        for (int i = 0; i < Button_TERM; ++i)
        {
            mBlankAfterSuspending[i] = mIsPressing[i]; // set a flag to ignore release event once
        }
    }
}

bool AbstractCursor::isLeftPressState() const
{
    return mEventType == Event_Press && mEventButton == Button_Left;
}

bool AbstractCursor::isLeftMoveState() const
{
    return mEventType == Event_Move && mIsPressing[Button_Left] && !mBlankAfterSuspending.at(Button_Left);
}

bool AbstractCursor::isLeftReleaseState() const
{
    return mEventType == Event_Release && mEventButton == Button_Left;
}

bool AbstractCursor::isRightPressState() const
{
    return mEventType == Event_Press && mEventButton == Button_Right;
}

bool AbstractCursor::isRightMoveState() const
{
    return mEventType == Event_Move && mIsPressing[Button_Right] && !mBlankAfterSuspending.at(Button_Right);
}

bool AbstractCursor::isRightReleaseState() const
{
    return mEventType == Event_Release && mEventButton == Button_Right;
}

bool AbstractCursor::isPressState() const
{
    return mEventType == Event_Press;
}

} // namespace core
