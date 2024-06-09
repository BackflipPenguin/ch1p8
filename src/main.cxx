#include <SFML/Graphics.hpp>
#include <iostream>
#include "include/Machine.hxx"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <rom file>" << std::endl;
        return -1;
    }

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    auto window = sf::RenderWindow{{800u, 600u}, "CMake SFML Project", sf::Style::Default, settings};
    window.setFramerateLimit(60);

    Machine machine;
    machine.load_rom(argv[1]);

    window.clear();
    window.display();
    while (window.isOpen())
    {
        for (auto event = sf::Event{}; window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }
        machine.key[0x1] = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Num1);
        machine.key[0x2] = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Num2);
        machine.key[0x3] = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Num3);
        machine.key[0xC] = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Num4);

        machine.key[0x4] = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Q);
        machine.key[0x5] = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::W);
        machine.key[0x6] = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::E);
        machine.key[0xD] = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::R);

        machine.key[0x7] = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::A);
        machine.key[0x8] = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::S);
        machine.key[0x9] = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::D);
        machine.key[0xE] = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::F);

        machine.key[0xA] = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Z);
        machine.key[0x0] = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::X);
        machine.key[0xB] = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::C);
        machine.key[0xF] = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::V);

        machine.cycle();
        if (machine.draw)
        {
            window.clear();
            for (auto y = 0; y < 32; ++y)
            {
                for (auto x = 0; x < 64; ++x)
                {
                    if (!machine.gfx[y * 64 + x])
                    {
                        continue;
                    }
                    auto rect = sf::RectangleShape{{10.f, 10.f}};
                    rect.setPosition(x * 10.f, y * 10.f);
                    rect.setFillColor(sf::Color::White);
                    window.draw(rect);
                }
            }
            window.display();
        }
    }
}