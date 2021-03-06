#include "SceneManager.h"

namespace rwe
{
    std::optional<MouseButtonEvent::MouseButton> convertSdlMouseButton(Uint8 button)
    {
        switch (button)
        {
            case SDL_BUTTON_LEFT:
                return MouseButtonEvent::MouseButton::Left;
            case SDL_BUTTON_MIDDLE:
                return MouseButtonEvent::MouseButton::Middle;
            case SDL_BUTTON_RIGHT:
                return MouseButtonEvent::MouseButton::Right;
            default:
                return std::nullopt;
        }
    }
    SceneManager::SceneManager(SdlContext* sdl, SDL_Window* window, GraphicsContext* graphics) : currentScene(), nextScene(), sdl(sdl), window(window), graphics(graphics), requestedExit(false)
    {
    }

    void SceneManager::setNextScene(std::shared_ptr<Scene> scene)
    {
        nextScene = std::move(scene);
    }

    void SceneManager::execute()
    {
        auto currentSimulationTime = sdl->getTicks();

        while (!requestedExit)
        {
            auto currentRealTime = sdl->getTicks();

            if (nextScene)
            {
                currentScene = std::move(nextScene);
                currentScene->init();
            }

            SDL_Event event;
            while (sdl->pollEvent(&event))
            {
                switch (event.type)
                {
                    case SDL_QUIT:
                        return;
                    case SDL_KEYDOWN:
                        currentScene->onKeyDown(event.key.keysym);
                        break;
                    case SDL_KEYUP:
                        currentScene->onKeyUp(event.key.keysym);
                        break;
                    case SDL_MOUSEBUTTONDOWN:
                    {
                        auto button = convertSdlMouseButton(event.button.button);
                        if (!button)
                        {
                            break;
                        }

                        MouseButtonEvent e(event.button.x, event.button.y, *button);
                        currentScene->onMouseDown(e);
                        break;
                    }
                    case SDL_MOUSEBUTTONUP:
                    {
                        auto button = convertSdlMouseButton(event.button.button);
                        if (!button)
                        {
                            break;
                        }

                        MouseButtonEvent e(event.button.x, event.button.y, *button);
                        currentScene->onMouseUp(e);
                        break;
                    }
                    case SDL_MOUSEMOTION:
                    {
                        MouseMoveEvent e(event.motion.x, event.motion.y);
                        currentScene->onMouseMove(e);
                        break;
                    }
                    case SDL_MOUSEWHEEL:
                    {
                        MouseWheelEvent e(event.wheel.x, event.wheel.y);
                        currentScene->onMouseWheel(e);
                        break;
                    }

                    default:
                        // skip unrecognised events
                        break;
                }
            }

            while (currentSimulationTime < currentRealTime)
            {
                currentScene->update();
                currentSimulationTime += TickInterval;
            }

            graphics->clear();
            currentScene->render(*graphics);
            sdl->glSwapWindow(window);

            auto finishTime = sdl->getTicks();
            auto nextSimTime = currentSimulationTime + TickInterval;
            if (finishTime < nextSimTime)
            {
                sdl->delay(nextSimTime - finishTime);
            }
        }
    }

    void SceneManager::requestExit()
    {
        requestedExit = true;
    }
}
