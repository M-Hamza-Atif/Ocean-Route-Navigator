#ifndef UICOMPONENTS_H
#define UICOMPONENTS_H

#include <SFML/Graphics.hpp>
#include <string>

// Validation states for InputBox
enum ValidationState {
    VALIDATION_NEUTRAL = 0,
    VALIDATION_VALID = 1,
    VALIDATION_INVALID = 2
};

class InputBox {
private:
    sf::RectangleShape m_box;
    sf::Text m_text;
    sf::Text m_label;
    sf::Text m_placeholderText;
    sf::Font& m_font;
    
    std::string m_buffer;
    std::string m_placeholder;
    
    float m_x, m_y;
    float m_width, m_height;
    
    bool m_focused;
    bool m_submitted;
    int m_validationState;
    
    // Cursor blinking
    sf::RectangleShape m_cursor;
    sf::Clock m_cursorClock;
    bool m_cursorVisible;
    
    // Colors
    sf::Color m_bgColor;
    sf::Color m_neutralBorder;
    sf::Color m_validBorder;
    sf::Color m_invalidBorder;
    sf::Color m_textColor;
    sf::Color m_placeholderColor;
    
    void updateVisuals() {
        // Update border color based on validation state
        switch (m_validationState) {
            case VALIDATION_VALID:
                m_box.setOutlineColor(m_validBorder);
                break;
            case VALIDATION_INVALID:
                m_box.setOutlineColor(m_invalidBorder);
                break;
            default:
                m_box.setOutlineColor(m_neutralBorder);
                break;
        }
        
        // Update text display
        m_text.setString(m_buffer);
        
        // Update cursor position
        float textWidth = m_text.getGlobalBounds().width;
        if (m_buffer.empty()) {
            textWidth = 0;
        }
        m_cursor.setPosition(m_x + 8 + textWidth + 2, m_y + 20 + 7);
    }

public:
    InputBox(float x, float y, float width, float height, const std::string& label, sf::Font& font)
        : m_font(font)
        , m_x(x)
        , m_y(y)
        , m_width(width)
        , m_height(height)
        , m_focused(false)
        , m_submitted(false)
        , m_validationState(VALIDATION_NEUTRAL)
        , m_cursorVisible(true)
        , m_bgColor(40, 40, 48, 255)
        , m_neutralBorder(100, 150, 255)
        , m_validBorder(0, 255, 0)
        , m_invalidBorder(255, 50, 50)
        , m_textColor(255, 255, 255)
        , m_placeholderColor(128, 128, 128)
    {
        // Setup label (above the input box)
        m_label.setFont(m_font);
        m_label.setString(label);
        m_label.setCharacterSize(12);
        m_label.setFillColor(sf::Color(200, 200, 200));
        m_label.setPosition(x, y);
        
        // Setup input box (below label)
        float boxY = y + 20; // Offset for label
        m_box.setSize(sf::Vector2f(width, height));
        m_box.setPosition(x, boxY);
        m_box.setFillColor(m_bgColor);
        m_box.setOutlineThickness(2.f);
        m_box.setOutlineColor(m_neutralBorder);
        
        // Setup text inside box (vertically centered)
        m_text.setFont(m_font);
        m_text.setCharacterSize(14);
        m_text.setFillColor(m_textColor);
        m_text.setPosition(x + 8, boxY + (height - 16) / 2.f);
        
        // Setup placeholder text
        m_placeholderText.setFont(m_font);
        m_placeholderText.setCharacterSize(14);
        m_placeholderText.setFillColor(m_placeholderColor);
        m_placeholderText.setPosition(x + 8, boxY + (height - 16) / 2.f);
        
        // Setup cursor
        m_cursor.setSize(sf::Vector2f(2, height - 14));
        m_cursor.setFillColor(sf::Color(100, 150, 255));
        m_cursor.setPosition(x + 8, boxY + 7);
        
        // Store actual box Y for later
        m_y = y;
    }
    
    void handleEvent(sf::Event& event) {
        float boxY = m_y + 20; // Label offset
        
        // Handle mouse clicks for focus
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                float mx = static_cast<float>(event.mouseButton.x);
                float my = static_cast<float>(event.mouseButton.y);
                
                sf::FloatRect bounds(m_x, boxY, m_width, m_height);
                if (bounds.contains(mx, my)) {
                    m_focused = true;
                    m_cursorClock.restart();
                    m_cursorVisible = true;
                } else {
                    m_focused = false;
                }
            }
        }
        
        // Handle text input when focused
        if (m_focused && event.type == sf::Event::TextEntered) {
            sf::Uint32 unicode = event.text.unicode;
            
            // Reset validation state when typing (except for Enter)
            if (unicode != 13) {
                m_validationState = VALIDATION_NEUTRAL;
            }
            
            if (unicode == 8) {
                // Backspace
                if (!m_buffer.empty()) {
                    m_buffer.pop_back();
                }
            } else if (unicode == 13) {
                // Enter/Return
                m_submitted = true;
            } else if (unicode >= 32 && unicode < 127) {
                // Printable ASCII characters
                m_buffer += static_cast<char>(unicode);
            }
            
            updateVisuals();
        }
    }
    
    void draw(sf::RenderWindow& window) {
        // Update cursor blink
        if (m_focused) {
            if (m_cursorClock.getElapsedTime().asMilliseconds() > 500) {
                m_cursorVisible = !m_cursorVisible;
                m_cursorClock.restart();
            }
        }
        
        updateVisuals();
        
        // Draw label
        window.draw(m_label);
        
        // Draw box
        window.draw(m_box);
        
        // Draw placeholder or text
        if (m_buffer.empty() && !m_placeholder.empty() && !m_focused) {
            window.draw(m_placeholderText);
        } else {
            window.draw(m_text);
        }
        
        // Draw cursor if focused
        if (m_focused && m_cursorVisible) {
            window.draw(m_cursor);
        }
    }
    
    void setPosition(float x, float y) {
        m_x = x;
        m_y = y;
        
        float boxY = y + 20;
        
        m_label.setPosition(x, y);
        m_box.setPosition(x, boxY);
        m_text.setPosition(x + 8, boxY + (m_height - 18) / 2);
        m_placeholderText.setPosition(x + 8, boxY + (m_height - 18) / 2);
        
        updateVisuals();
    }
    
    void clear() {
        m_buffer.clear();
        m_submitted = false;
        m_validationState = VALIDATION_NEUTRAL;
        updateVisuals();
    }
    
    std::string getText() const {
        return m_buffer;
    }
    
    void setText(const std::string& text) {
        m_buffer = text;
        updateVisuals();
    }
    
    bool isFocused() const {
        return m_focused;
    }
    
    bool wasSubmitted() {
        if (m_submitted) {
            m_submitted = false;
            return true;
        }
        return false;
    }
    
    void setValidationState(int state) {
        m_validationState = state;
        updateVisuals();
    }
    
    void resetValidation() {
        m_validationState = VALIDATION_NEUTRAL;
        updateVisuals();
    }
    
    void setPlaceholder(const std::string& placeholder) {
        m_placeholder = placeholder;
        m_placeholderText.setString(placeholder);
    }
};


class ToggleSwitch {
private:
    sf::RectangleShape m_background;
    sf::RectangleShape m_leftButton;
    sf::RectangleShape m_rightButton;
    sf::RectangleShape m_highlight;
    
    sf::Text m_leftLabel;
    sf::Text m_rightLabel;
    sf::Font& m_font;
    
    float m_x, m_y;
    float m_width, m_height;
    
    bool m_leftSelected;
    
    // Colors
    sf::Color m_bgColor;
    sf::Color m_selectedColor;
    sf::Color m_unselectedColor;
    sf::Color m_borderColor;
    sf::Color m_selectedTextColor;
    sf::Color m_unselectedTextColor;
    
    void updateVisuals() {
        float halfWidth = m_width / 2.f;
        
        if (m_leftSelected) {
            // Left is selected
            m_leftButton.setFillColor(m_selectedColor);
            m_rightButton.setFillColor(m_unselectedColor);
            m_leftLabel.setFillColor(m_selectedTextColor);
            m_rightLabel.setFillColor(m_unselectedTextColor);
        } else {
            // Right is selected
            m_leftButton.setFillColor(m_unselectedColor);
            m_rightButton.setFillColor(m_selectedColor);
            m_leftLabel.setFillColor(m_unselectedTextColor);
            m_rightLabel.setFillColor(m_selectedTextColor);
        }
        
        // Center text in each half
        centerText(m_leftLabel, m_x, m_y, halfWidth, m_height);
        centerText(m_rightLabel, m_x + halfWidth, m_y, halfWidth, m_height);
    }
    
    void centerText(sf::Text& text, float boxX, float boxY, float boxW, float boxH) {
        sf::FloatRect textBounds = text.getLocalBounds();
        float textX = std::floor(boxX + (boxW - textBounds.width) / 2.f - textBounds.left);
        float textY = std::floor(boxY + (boxH - textBounds.height) / 2.f - textBounds.top);
        text.setPosition(textX, textY);
    }

public:
    ToggleSwitch(float x, float y, float width, float height,
                 const std::string& labelLeft, const std::string& labelRight,
                 sf::Font& font)
        : m_font(font)
        , m_x(x)
        , m_y(y)
        , m_width(width)
        , m_height(height)
        , m_leftSelected(true)
        , m_bgColor(40, 40, 48, 255)
        , m_selectedColor(60, 120, 200, 100)
        , m_unselectedColor(40, 40, 48, 255)
        , m_borderColor(100, 150, 255)
        , m_selectedTextColor(255, 255, 255)
        , m_unselectedTextColor(128, 128, 128)
    {
        float halfWidth = width / 2.f;
        
        // Setup background (full control)
        m_background.setSize(sf::Vector2f(width, height));
        m_background.setPosition(x, y);
        m_background.setFillColor(m_bgColor);
        m_background.setOutlineThickness(2.f);
        m_background.setOutlineColor(m_borderColor);
        
        // Setup left button
        m_leftButton.setSize(sf::Vector2f(halfWidth, height));
        m_leftButton.setPosition(x, y);
        m_leftButton.setFillColor(m_selectedColor);
        
        // Setup right button
        m_rightButton.setSize(sf::Vector2f(halfWidth, height));
        m_rightButton.setPosition(x + halfWidth, y);
        m_rightButton.setFillColor(m_unselectedColor);
        
        // Setup left label
        m_leftLabel.setFont(m_font);
        m_leftLabel.setString(labelLeft);
        m_leftLabel.setCharacterSize(16);
        m_leftLabel.setFillColor(m_selectedTextColor);
        
        // Setup right label
        m_rightLabel.setFont(m_font);
        m_rightLabel.setString(labelRight);
        m_rightLabel.setCharacterSize(16);
        m_rightLabel.setFillColor(m_unselectedTextColor);
        
        updateVisuals();
    }
    
    void handleEvent(sf::Event& event) {
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                float mx = static_cast<float>(event.mouseButton.x);
                float my = static_cast<float>(event.mouseButton.y);
                
                float halfWidth = m_width / 2.f;
                
                // Check left button
                sf::FloatRect leftBounds(m_x, m_y, halfWidth, m_height);
                if (leftBounds.contains(mx, my)) {
                    m_leftSelected = true;
                    updateVisuals();
                    return;
                }
                
                // Check right button
                sf::FloatRect rightBounds(m_x + halfWidth, m_y, halfWidth, m_height);
                if (rightBounds.contains(mx, my)) {
                    m_leftSelected = false;
                    updateVisuals();
                    return;
                }
            }
        }
    }
    
    void draw(sf::RenderWindow& window) {
        // Draw buttons first (they form the filled background)
        window.draw(m_leftButton);
        window.draw(m_rightButton);
        
        // Draw outer border only (make background transparent so buttons show through)
        m_background.setFillColor(sf::Color::Transparent);
        window.draw(m_background);
        
        // Draw separator line in the middle
        sf::RectangleShape separator;
        separator.setSize(sf::Vector2f(2.f, m_height));
        separator.setPosition(m_x + m_width / 2.f - 1.f, m_y);
        separator.setFillColor(m_borderColor);
        window.draw(separator);
        
        // Draw labels
        window.draw(m_leftLabel);
        window.draw(m_rightLabel);
    }
    
    void setPosition(float x, float y) {
        m_x = x;
        m_y = y;
        
        float halfWidth = m_width / 2.f;
        
        m_background.setPosition(x, y);
        m_leftButton.setPosition(x, y);
        m_rightButton.setPosition(x + halfWidth, y);
        
        updateVisuals();
    }
    
    bool isLeftSelected() const {
        return m_leftSelected;
    }
    
    void setSelection(bool left) {
        m_leftSelected = left;
        updateVisuals();
    }
};

#endif
