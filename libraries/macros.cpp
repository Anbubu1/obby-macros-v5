#define __MacroDebug

#include <algorithm>
#include <windows.h>
#include <iostream>

#include <globals.hpp>
#include <macros.hpp>
#include <tasks.hpp>

using Globals::FloatSliderFlags;
using Globals::IntSliderFlags;
using Globals::BooleanFlags;

using Globals::ROBLOX_SENS_MULT;

void MouseFlick(const int Angle, const bool NoReturn) {
    static std::atomic<float>* const FlickSensitivity = &FloatSliderFlags["Flick Sensitivity"];
    static std::atomic<bool>* const HumanLikeFlick = &BooleanFlags["Human-like Flicking"];
    static std::atomic<int>* const FlickDuration = &IntSliderFlags["Flick Duration"];
    static std::atomic<int>* const FlickDelay = &IntSliderFlags["Flick Delay"];

    INPUT Input = { 0 };
    Input.type = INPUT_MOUSE;

    MOUSEINPUT* const MouseInput = &Input.mi;
    MouseInput->dwFlags = MOUSEEVENTF_MOVE;

    const double Delay = 1.0 / static_cast<double>(FlickDelay->load());

#ifdef __MacroDebug
    LONG PixelsTravelled = 0;
#endif

    if (HumanLikeFlick->load()) {
        const LONG TotalPixels = Angle * ROBLOX_SENS_MULT * FlickSensitivity->load();

        const double Duration = 1.0 / static_cast<double>(FlickDuration->load());

        const auto DoPhase = [&](const bool Reversed) {
            LONG Moved = 0;
            double ElapsedTime = 0.0;
            const double PhaseDuration = Duration / 2.0;

            if (PhaseDuration <= 0.0) {
                LONG rem = TotalPixels;
                MouseInput->dx = Reversed ? -rem : rem;
                SendInput(1, &Input, sizeof(INPUT));
#ifdef __MacroDebug
                PixelsTravelled += rem;
#endif
                return;
            }

            while (Moved < TotalPixels) {
                ElapsedTime += ShortWait();

                double t = std::clamp(ElapsedTime / PhaseDuration, 0.0, 1.0);

                const double Smoothed = 1 - (1 - t) * (1 - t) * (1 - t);

                const LONG Target = static_cast<LONG>(std::round(Smoothed * static_cast<double>(TotalPixels)));
                const LONG DeltaPixels = Target - Moved;

                if (DeltaPixels > 0) {
                    MouseInput->dx = Reversed ? -DeltaPixels : DeltaPixels;
                    SendInput(1, &Input, sizeof(INPUT));
                    Moved += DeltaPixels;
#ifdef __MacroDebug
                    PixelsTravelled += DeltaPixels;
#endif
                }

                if (t >= 1.0 && Moved < TotalPixels) {
                    const LONG rem = TotalPixels - Moved;
                    if (rem > 0) {
                        MouseInput->dx = Reversed ? -rem : rem;
                        SendInput(1, &Input, sizeof(INPUT));
                        Moved += rem;
#ifdef __MacroDebug
                        PixelsTravelled += rem;
#endif
                    }
                    break;
                }
            }

#ifdef __MacroDebug
            std::cout << std::to_string(PixelsTravelled) << std::endl;
#endif
        };

        DoPhase(false);
        if (NoReturn) return;

        Wait(Delay);
        DoPhase(true);
    } else {
        const LONG Pixels = Angle * ROBLOX_SENS_MULT * FlickSensitivity->load();

        MouseInput->dx = Pixels;
        SendInput(1, &Input, sizeof(INPUT));
        if (NoReturn) return;

        Wait(Delay);

        MouseInput->dx = -Pixels;
        SendInput(1, &Input, sizeof(INPUT));
    }
}