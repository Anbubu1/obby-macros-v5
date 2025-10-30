#undef __MacroDebug

#include <algorithm>
#include <windows.h>

#ifdef __MacroDebug
    #include <iostream>
#endif

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

    INPUT Input{};
    Input.type = INPUT_MOUSE;

    MOUSEINPUT* const MouseInput = &Input.mi;
    MouseInput->dwFlags = MOUSEEVENTF_MOVE;

    const double Delay = 1.0 / static_cast<double>(FlickDelay->load());

#ifdef __MacroDebug
    LONG PixelsTravelled = 0;
#endif

    if (HumanLikeFlick->load()) {
        const float TotalPixels = static_cast<float>(Angle) * ROBLOX_SENS_MULT * FlickSensitivity->load();

        const float Duration = 1.0f / static_cast<float>(FlickDuration->load());

        const auto DoPhase = [&](const bool Reversed) {
            float Moved = 0.0f;
            float ElapsedTime = 0.0f;
            const float PhaseDuration = Duration / 2.0f;

            if (PhaseDuration <= 0.0) {
                float rem = TotalPixels;
                MouseInput->dx = static_cast<long>(Reversed ? -rem : rem);
                SendInput(1, &Input, sizeof(INPUT));
#ifdef __MacroDebug
                PixelsTravelled += rem;
#endif
                return;
            }

            while (Moved < TotalPixels) {
                ElapsedTime += ShortWait();

                const float t = std::clamp(ElapsedTime / PhaseDuration, 0.0f, 1.0f);

                const float Smoothed = 1 - (1 - t) * (1 - t) * (1 - t);

                const float Target = static_cast<float>(std::round(Smoothed * static_cast<float>(TotalPixels)));
                const float DeltaPixels = Target - Moved;

                if (DeltaPixels > 0.0f) {
                    MouseInput->dx = static_cast<long>(Reversed ? -DeltaPixels : DeltaPixels);
                    SendInput(1, &Input, sizeof(INPUT));
                    Moved += DeltaPixels;
#ifdef __MacroDebug
                    PixelsTravelled += DeltaPixels;
#endif
                }

                if (t >= 1.0f && Moved < TotalPixels) {
                    const float rem = TotalPixels - Moved;
                    if (rem > 0) {
                        MouseInput->dx = static_cast<long>(Reversed ? -rem : rem);
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
        const long Pixels = static_cast<long>(static_cast<float>(Angle) * ROBLOX_SENS_MULT * FlickSensitivity->load());

        MouseInput->dx = Pixels;
        SendInput(1, &Input, sizeof(INPUT));
        if (NoReturn) return;

        Wait(Delay);

        MouseInput->dx = -Pixels;
        SendInput(1, &Input, sizeof(INPUT));
    }
}