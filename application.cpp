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
          m_changeColorButtonText(m_font),
          m_resetZoomingButtonText(m_font)
    {
        //Setting the SMFL Image and texture
        m_image.resize(sf::Vector2u{static_cast<unsigned int>(m_fractal.WIDTH),
                                    static_cast<unsigned int>(m_fractal.HEIGHT)});
        drawFractal();
        m_texture.loadFromImage(m_image);
        m_sprite.setTexture(m_texture);

        //Rectange that will be used to zoom in
        m_selectionRect.setFillColor(sf::Color(0, 0, 0, 0));
        m_selectionRect.setOutlineColor(sf::Color::Red);
        m_selectionRect.setOutlineThickness(3);

        //Load Font
        if (!m_font.openFromFile("TimesNewRoman.ttf"))
        {
            std::cout << "Please, provide a Font!\n";
        }
        createButton(width - 150, 5, m_changeColorButton, m_changeColorButtonText,
                     "Change color");
        createButton(width - 150, 50, m_resetZoomingButton,
                     m_resetZoomingButtonText, "Reset Zooming");
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
            if (m_dragging)
            {
                m_window.draw(m_selectionRect);
            }
            m_window.draw(m_changeColorButton);
            m_window.draw(m_changeColorButtonText);
            m_window.draw(m_resetZoomingButton);
            m_window.draw(m_resetZoomingButtonText);

            m_window.display();
        }
    }

private:
    void drawFractal()
    {
        m_fractal.renderFract();
        for (int y = 0; y < m_fractal.HEIGHT; ++y)
        {
            for (int x = 0; x < m_fractal.WIDTH; ++x)
            {
                int iter = m_fractal.image[y * m_fractal.WIDTH + x];
                sf::Color color = getColor(iter);
                m_image.setPixel(sf::Vector2u{static_cast<unsigned int>(x),
                                              static_cast<unsigned int>(y)},
                                 color);
            }
        }
    }

    sf::Color getColor(int iter)
    {
        if (iter < m_fractal.MAX_ITR)
        {
            switch (m_COLOR)
            {
            case 1:
                return sf::Color(0, 255 * iter / m_fractal.MAX_ITR,
                                 255 * iter / m_fractal.MAX_ITR);
            case 2:
                return sf::Color(255 * iter / m_fractal.MAX_ITR, 0,
                                 255 * iter / m_fractal.MAX_ITR);
            case 3:
                return sf::Color(255 * iter / m_fractal.MAX_ITR,
                                 255 * iter / m_fractal.MAX_ITR, 0);
            }
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
                if (m_changeColorButton.getGlobalBounds().contains(mousePosition))
                {
                    changeColorButtonHandler();
                }
                else if (m_resetZoomingButton.getGlobalBounds().contains(mousePosition))
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

        if (const auto *mouseWheelScrolled = event.getIf<sf::Event::MouseWheelScrolled>())
        {
            if (mouseWheelScrolled->delta != 0)
            {
                ZoomByScrolling(mouseWheelScrolled->delta, mouseWheelScrolled->position.x,
                                mouseWheelScrolled->position.y);
            }
        }
    }

    void changeColorButtonHandler()
    {
        m_COLOR = (m_COLOR % 3) + 1;
        drawFractal();
        m_texture.loadFromImage(m_image);
        m_sprite.setTexture(m_texture);
    }

    void resetZoomingButtonHandler()
    {
        m_fractal.defaultRegion();
        drawFractal();
        m_texture.loadFromImage(m_image);
        m_sprite.setTexture(m_texture);
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

        m_fractal.updateRegion(realMin, realMax, imagMin, imagMax);
        drawFractal();
        m_texture.loadFromImage(m_image);
        m_sprite.setTexture(m_texture);
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
        m_texture.loadFromImage(m_image);
        m_sprite.setTexture(m_texture);
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

    sf::RectangleShape m_changeColorButton;
    sf::RectangleShape m_resetZoomingButton;
    sf::Font m_font;
    sf::Text m_changeColorButtonText{m_font};
    sf::Text m_resetZoomingButtonText{m_font};

    constexpr static const double EPS = 1e-10;
    int m_COLOR = 1; // Color palette identifier (1, 2, or 3)
    bool m_dragging =
        false; // Flag to indicate if the user is dragging for selection
};