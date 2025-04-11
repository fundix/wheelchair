#ifndef WHEEL_CONTROLLER_HPP
#define WHEEL_CONTROLLER_HPP


class WheelController
{

public:
    WheelController();
    void setSpeed(int leftSpeed, int rightSpeed);
    void stop();
    void setDirection(bool leftReverse, bool rightReverse);

};
#endif // WHEEL_CONTROLLER_HPP