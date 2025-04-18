#ifndef ANGLE_H
#define ANGLE_H

#include <string>

class Angle {
private:
    std::string name;
    int degrees;
    int minutes;
    int seconds;

public:

    Angle() : name("RandomAngle"), degrees(rand() % 360), minutes(rand() % 60), seconds(rand() % 60) {}

    Angle(const std::string& myName, int myDegrees, int myMinutes, int mySeconds)
        : name(myName), degrees(myDegrees), minutes(myMinutes), seconds(mySeconds) {}

    Angle(int totalSeconds) : name("AngleFromSeconds") {
        degrees = totalSeconds / 3600;
        totalSeconds %= 3600;
        minutes = totalSeconds / 60;
        seconds = totalSeconds % 60;
    }


    Angle(const Angle& other) : name(other.name), degrees(other.degrees), minutes(other.minutes), seconds(other.seconds) {}

    ~Angle() {}

    void In();
    void Out() const;
    int ToSeconds() const;
};

#endif