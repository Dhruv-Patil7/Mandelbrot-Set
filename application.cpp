#include <iomanip>
#include <chrono>
#include <sstream>
#include <SFML/Graphics.hpp>
#include <complex>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include "mandelbrot.cpp"

class Application
{
public:
    Application(int width, int height, int maxIter, int numThreads)
        : m_fractal(MandelbrotSet(width, height, maxIter, numThreads)),
          m_window(sf::VideoMode(sf::Vector2u{static_cast<unsigned int>(width),
                                              static_cast<unsigned int>(height)}),
                   "Mandelbrot Set", sf::Style::Titlebar | sf::Style::Close),
          m_sprite(m_texture),
          m_resetZoomingButtonText(m_font)
    {
        //Setting the SMFL Image and texture
        m_image.resize(sf::Vector2u{static_cast<unsigned int>(m_fractal.WIDTH),
                                    static_cast<unsigned int>(m_fractal.HEIGHT)});
        drawFractal();
        static_cast<void>(m_texture.loadFromImage(m_image));
        m_sprite.setTexture(m_texture, true);

        //Rectange that will be used to zoom in
        m_selectionRect.setFillColor(sf::Color(0, 0, 0, 0));
        m_selectionRect.setOutlineColor(sf::Color::Red);
        m_selectionRect.setOutlineThickness(3);

        //Load Font
        if (!m_font.openFromFile("TimesNewRoman.ttf"))
        {
            std::cout << "Find yourself a font Human!\n";
        }
        m_hudText.setFont(m_font);
        m_hudText.setCharacterSize(16);
        m_hudText.setFillColor(sf::Color(230, 230, 230));
        m_hudText.setPosition(sf::Vector2f(15.f, 15.f));
        m_hudBackground.setFillColor(sf::Color(0,0,0,170));
        updateHUD();
        createButton(width - 150, 50,
             m_resetZoomingButton,
             m_resetZoomingButtonText,
             "Reset Zooming");
    }

    void run()
    {
        while (m_window.isOpen())
        {
            while (const auto event = m_window.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                {
                    m_window.close();
                    return;// If window close is pressed the function returns
                }
                handleEvent(*event);
            }
            m_window.draw(m_sprite);
            m_window.draw(m_hudBackground);
            m_window.draw(m_hudText);
            if (m_dragging)
            {
                m_window.draw(m_selectionRect);
            }
            m_window.draw(m_resetZoomingButton);
            m_window.draw(m_resetZoomingButtonText);

            m_window.display();
        }
    }

private:
    void updateHUD()
    {
        std::ostringstream zoomStream;
        zoomStream << std::fixed << std::setprecision(2) << m_zoom;
        std::ostringstream renderStream;
        renderStream << std::fixed << std::setprecision(2)
             << m_renderTime;
       
        std::string fractalName =
            (m_fractal.getFractalType() == FractalType::Mandelbrot)
                ? "Mandelbrot"
                : "Julia";

        std::string hud =
            fractalName +
            "\nZoom       : " + zoomStream.str() + "x";

        if (m_fractal.getFractalType() == FractalType::Julia)
        {
            hud += "\nPreset     : " +
                std::to_string(m_fractal.getJuliaPreset());
        }
        // Putting Fun element in HUD
        hud +=
            "\nIterations : " + std::to_string(m_fractal.MAX_ITR) +
            "\nThreads    : " + std::to_string(std::thread::hardware_concurrency()) +
            "\nRender Time: " + renderStream.str() + "ms";

        m_hudText.setString(hud);

        sf::FloatRect bounds = m_hudText.getLocalBounds();

        m_hudBackground.setPosition(
            m_hudText.getPosition() - sf::Vector2f(8.f, 8.f));

        m_hudBackground.setSize(
            sf::Vector2f(bounds.size.x + 16.f,
                        bounds.size.y + 16.f));
    }
    void drawFractal()
    {
        auto start = std::chrono::high_resolution_clock::now();

        m_fractal.renderFract();
        for (int y = 0; y < m_fractal.HEIGHT; ++y)
        {
            for (int x = 0; x < m_fractal.WIDTH; ++x)
            {
                double iter = m_fractal.image[y * m_fractal.WIDTH + x];
                sf::Color color = getColor(iter);// Gets No. of iters for each pixels
                m_image.setPixel(sf::Vector2u{static_cast<unsigned int>(x),
                                              static_cast<unsigned int>(y)},
                                 color);
            }
        }
        auto end = std::chrono::high_resolution_clock::now();

    m_renderTime = std::chrono::duration<double, std::milli>(end - start).count();
    }

    sf::Color getColor(double iter)
    {
        if (iter < m_fractal.MAX_ITR)
        {
            constexpr double PALETTE_REPEAT = 6.0;
            double t = iter / m_fractal.MAX_ITR * PALETTE_REPEAT;
            t = t - std::floor(t);// Calculating t to get number between 0 - 1
            double position = t * (palette.size() - 1);
            //finding the color the pixel will lie between
            int left = std::floor(position);
            int right = std::min(left + 1, (int)palette.size() - 1);
            
            double fraction = position - left;
            sf::Color c1 = palette[left];
            sf::Color c2 = palette[right];

            // Calculating the RGB values for color scheme
            double r = c1.r * (1 - fraction) + c2.r * fraction;
            double g = c1.g * (1 - fraction) + c2.g * fraction;
            double b = c1.b * (1 - fraction) + c2.b * fraction;

            return sf::Color(r, g, b);
        }
        return sf::Color::Black;
    }

    void handleEvent(const sf::Event &event)
    {
        if (const auto *mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if (mouseButtonPressed->button == sf::Mouse::Button::Left)
            {
                const sf::Vector2f mousePosition{
                    static_cast<float>(mouseButtonPressed->position.x),
                    static_cast<float>(mouseButtonPressed->position.y)};
                if (m_resetZoomingButton.getGlobalBounds().contains(mousePosition))
                {
                    resetZoomingButtonHandler();
                }
                else
                {
                    // Rectangle is created
                    m_dragging = true;
                    m_start = sf::Mouse::getPosition(m_window);
                }
            }
            return;
        }

        // Until this Rectangle is recorded
        if (const auto *mouseButtonReleased = event.getIf<sf::Event::MouseButtonReleased>())
        {
            if (mouseButtonReleased->button == sf::Mouse::Button::Left && m_dragging)
            {
                handleZoomingIn();
            }
            return;
        }

        if (m_dragging)
        {
            handleDragging();
            return;
        }

        if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>())
        {
            // Toggle Mandelbrot / Julia
            if (keyPressed->code == sf::Keyboard::Key::J)
            {
                if (m_fractal.getFractalType() == FractalType::Mandelbrot){
                    m_fractal.setFractalType(FractalType::Julia);
                    m_window.setTitle("Julia Set");// Updating the name accordingly
                }
                else{
                    m_fractal.setFractalType(FractalType::Mandelbrot);
                    m_window.setTitle("Mandelbrot Set");
                }

                m_fractal.defaultRegion();
                m_zoom = 1.0;

                drawFractal();
                static_cast<void>(m_texture.loadFromImage(m_image));
                m_sprite.setTexture(m_texture, true);

                updateHUD();
            }

            // Julia presets
            if (m_fractal.getFractalType() == FractalType::Julia)
            {
                switch (keyPressed->code)
                {
                case sf::Keyboard::Key::Num1:
                    m_fractal.setJuliaPreset(1);
                    break;

                case sf::Keyboard::Key::Num2:
                    m_fractal.setJuliaPreset(2);
                    break;

                case sf::Keyboard::Key::Num3:
                    m_fractal.setJuliaPreset(3);
                    break;

                case sf::Keyboard::Key::Num4:
                    m_fractal.setJuliaPreset(4);
                    break;

                case sf::Keyboard::Key::Num5:
                    m_fractal.setJuliaPreset(5);
                    break;

                default:
                    return;
                }

                drawFractal();
                static_cast<void>(m_texture.loadFromImage(m_image));
                m_sprite.setTexture(m_texture, true);

                updateHUD();
            }

            return;
        }

        if (const auto *mouseWheelScrolled = event.getIf<sf::Event::MouseWheelScrolled>())
        {
            if (mouseWheelScrolled->delta != 0)
            {
                ZoomByScrolling(mouseWheelScrolled->delta, mouseWheelScrolled->position.x,
                                mouseWheelScrolled->position.y);
            }
        }
    }

    void resetZoomingButtonHandler()
    {
        m_fractal.defaultRegion();
        m_zoom = 1.0;
        updateHUD();

        drawFractal();
        static_cast<void>(m_texture.loadFromImage(m_image));
        m_sprite.setTexture(m_texture, true);
    }

    void handleZoomingIn()
    {
        m_dragging = false;
        m_end = sf::Mouse::getPosition(m_window);
        if (std::abs(m_start.x - m_end.x) < EPS &&
            std::abs(m_start.y - m_end.y) < EPS)
        {
            return;
        }
        // To preserve the aspect ratio 4:3
        double widthRatio = 4.0 / 3.0;
        double newWidth = m_end.x - m_start.x;
        double newHeight = newWidth / widthRatio;

        if (newHeight < 0)
            newHeight = -newHeight;

        // Changing the end point as required
        if (m_end.y < m_start.y)
        {
            m_end.y = m_start.y - newHeight;
        }
        else
        {
            m_end.y = m_start.y + newHeight;
        }

        // Creating new area on complex plane
        double realMin =
            m_fractal.REAL_MIN + (m_fractal.REAL_MAX - m_fractal.REAL_MIN) *
                                     std::min(m_start.x, m_end.x) / m_fractal.WIDTH;
        double realMax =
            m_fractal.REAL_MIN + (m_fractal.REAL_MAX - m_fractal.REAL_MIN) *
                                     std::max(m_start.x, m_end.x) / m_fractal.WIDTH;
        double imagMin = m_fractal.IMAGE_MIN +
                         (m_fractal.IMAGE_MAX - m_fractal.IMAGE_MIN) *
                             std::min(m_start.y, m_end.y) / m_fractal.HEIGHT;
        double imagMax = m_fractal.IMAGE_MIN +
                         (m_fractal.IMAGE_MAX - m_fractal.IMAGE_MIN) *
                             std::max(m_start.y, m_end.y) / m_fractal.HEIGHT;
        // removing the rectange
        m_selectionRect.setSize(sf::Vector2f(0, 0));

        double old = m_fractal.REAL_MAX - m_fractal.REAL_MIN;
        double neew = realMax - realMin;
        m_zoom *= old / neew;
        m_fractal.updateRegion(realMin, realMax, imagMin, imagMax);
        updateHUD();

        drawFractal();
        static_cast<void>(m_texture.loadFromImage(m_image));
        m_sprite.setTexture(m_texture, true);
    }

    void handleDragging()
    {
        // Update selection rectangle during dragging
        m_end = sf::Mouse::getPosition(m_window);

        double newWidth = std::abs(m_end.x - m_start.x);
        double newHeight =
            newWidth * 3.0 / 4.0; // Adjust the aspect ratio (must be 4:3)

        if (m_end.x > m_start.x && m_end.y < m_start.y)
        { // up-right
            m_end.y = m_start.y - newHeight;
        }
        else if (m_end.x < m_start.x && m_end.y < m_start.y)
        { // up-left
            m_end.y = m_start.y - newHeight;
            m_end.x = m_start.x - newWidth;
        }
        else if (m_end.x < m_start.x && m_end.y > m_start.y)
        { // down-left
            m_end.x = m_start.x - newWidth;
        }
        else
        { // down-right
            m_end.y = m_start.y + newHeight;
        }

        m_selectionRect.setPosition(sf::Vector2f(std::min(m_start.x, m_end.x),
                                                 std::min(m_start.y, m_end.y)));
        m_selectionRect.setSize(sf::Vector2f(newWidth, newHeight));
    }

    void ZoomByScrolling(float delta, int mouseX, int mouseY)
    {
        // Calculate zoom factor
        double zoomFactor = (delta > 0) ? 0.9 : 1.1;
        m_zoom /= zoomFactor;
        updateHUD();

        // Calculate the new complex plane coordinates based on mouse position
        double mouseRe =
            m_fractal.REAL_MIN +
            (m_fractal.REAL_MAX - m_fractal.REAL_MIN) * mouseX / m_fractal.WIDTH;
        double mouseIm =
            m_fractal.IMAGE_MIN +
            (m_fractal.IMAGE_MAX - m_fractal.IMAGE_MIN) * mouseY / m_fractal.HEIGHT;

        // Zoom in or out by adjusting the region
        double newWidth = (m_fractal.REAL_MAX - m_fractal.REAL_MIN) * zoomFactor;
        double newHeight = (m_fractal.IMAGE_MAX - m_fractal.IMAGE_MIN) * zoomFactor;

        // Center the new region around the mouse position
        double newRealMin = mouseRe - (mouseRe - m_fractal.REAL_MIN) * zoomFactor;
        double newRealMax = newRealMin + newWidth;
        double newImagMin = mouseIm - (mouseIm - m_fractal.IMAGE_MIN) * zoomFactor;
        double newImagMax = newImagMin + newHeight;

        m_fractal.updateRegion(newRealMin, newRealMax, newImagMin, newImagMax);
        drawFractal();
        static_cast<void>(m_texture.loadFromImage(m_image));
        m_sprite.setTexture(m_texture, true);
    }

    void createButton(int x, int y, sf::RectangleShape &button, sf::Text &text,
                      const std::string &title)
    {
        button = sf::RectangleShape(sf::Vector2f(125, 30));
        button.setFillColor(sf::Color::Black);
        button.setPosition(sf::Vector2f(static_cast<float>(x), static_cast<float>(y)));
        button.setOutlineThickness(2);
        button.setOutlineColor(sf::Color::White);

        text.setFont(m_font);
        text.setString(title);
        text.setCharacterSize(15);
        text.setFillColor(sf::Color::White);
        text.setPosition(sf::Vector2f(button.getPosition().x + 10.f,
                                      button.getPosition().y + 5.f));
    }

    MandelbrotSet m_fractal;
    sf::RenderWindow m_window;

    sf::Image m_image;
    sf::Texture m_texture;
    sf::Sprite m_sprite;

    sf::Vector2i m_start;
    sf::Vector2i m_end;
    sf::RectangleShape m_selectionRect;

    sf::RectangleShape m_resetZoomingButton;
    sf::Font m_font;
    sf::RectangleShape m_hudBackground;
    sf::Text m_hudText{m_font};
    sf::Text m_resetZoomingButtonText{m_font};
    std::vector<sf::Color> palette = {
        sf::Color(0,   7,   100),   // Deep Blue
        sf::Color(12,  44,  138),
        sf::Color(24,  82,  177),
        sf::Color(57,  125, 209),
        sf::Color(134, 181, 229),
        sf::Color(211, 236, 248),
        sf::Color(241, 250, 255),   // Almost White
        sf::Color(255, 255, 255),   // White
        sf::Color(255, 238, 170),
        sf::Color(255, 215, 85),
        sf::Color(255, 170, 0),
        sf::Color(230, 135, 0),
        sf::Color(190, 95,  0),
        sf::Color(140, 70,  0),
        sf::Color(90,  40,  0),
        sf::Color(0,   7,   100)    // Loop back to Deep Blue
    };

    constexpr static const double EPS = 1e-10;
    bool m_dragging = false; // Flag to indicate if the user is dragging for selection
    double m_zoom = 1.0; // Initially set to zero
    double m_renderTime = 0.0;
};