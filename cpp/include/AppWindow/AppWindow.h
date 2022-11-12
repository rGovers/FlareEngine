#pragma once

class AppWindow
{
private:

protected:

public:
    AppWindow() { }
    virtual ~AppWindow() { }

    virtual bool ShouldClose() const = 0;

    virtual double GetDelta() const = 0;
    virtual double GetTime() const = 0;

    virtual void Update() = 0;
};