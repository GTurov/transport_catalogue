#pragma once

#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg {

using namespace std::literals;

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {}

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indentStep(indent_step)
        , indent(indent) {}

    RenderContext Indented() const {
        return {out, indentStep, indent + indentStep};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indentStep = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

class ObjectContainer {
public:
    template <typename Obj>
    void Add(Obj obj) {
        AddPtr(std::make_unique<Obj>(std::move(obj)));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    virtual ~ObjectContainer() = default;
};

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;

    virtual ~Drawable() = default;
};

class Document: public ObjectContainer {
public:

    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

    // Прочие методы и данные, необходимые для реализации класса Document

private:
    std::vector<std::unique_ptr<Object>> objects_;
};


struct Rgb {
    Rgb() = default;
    Rgb(uint8_t red, uint8_t green, uint8_t blue)
        :red(red), green(green), blue(blue){}
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

inline std::ostream& operator<<(std::ostream& out, Rgb color) {
    using namespace std::literals;
    out<<"rgb("sv<<+color.red<<","sv<<+color.green<<","sv<<+color.blue<<")"sv;
    return out;
}


struct Rgba {
    Rgba() = default;
    Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity)
        :red(red), green(green), blue(blue), opacity(opacity) {}
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};

inline std::ostream& operator<<(std::ostream& out, Rgba color) {

    out<<"rgba("sv<<+color.red<<","sv<<+color.green<<","sv<<+color.blue<<","sv<<+color.opacity<<")"sv;
    return out;
}

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

// Объявив в заголовочном файле константу со спецификатором inline,
// мы сделаем так, чтобы она была одной на все единицы трансляции,
// которые подключают этот заголовок.
// В противном случае каждая единица трансляции будет использовать свою копию этой константы
inline const Color NoneColor {};

struct OstreamColorPrinter {
    std::ostream& out;
    void operator()(std::monostate) const {
        out << "none"sv;
    }
    void operator()(std::string color) const {
        out << color;
    }
    void operator()(Rgb color) const {
        out << color;
    }
    void operator()(Rgba color) const {
        out << color;
    }
};

inline std::ostream& operator<<(std::ostream& out, Color color) {
    if (std::holds_alternative<std::monostate>(color)) {
        out << "none"sv;
    } else if (std::holds_alternative<std::string>(color)) {
        out << std::get<std::string>(color);
    }else if (std::holds_alternative<Rgb>(color)) {
        out << std::get<Rgb>(color);
    }else if (std::holds_alternative<Rgba>(color)) {
                out << std::get<Rgba>(color);
    }

    return out;
}

namespace detail {

inline uint8_t Lerp(uint8_t from, uint8_t to, double t) {
    return static_cast<uint8_t>(std::round((to - from) * t + from));
}

} // detail

inline svg::Rgb Lerp(svg::Rgb from, svg::Rgb to, double t) {
    using namespace detail;
    return {Lerp(from.red, to.red, t), Lerp(from.green, to.green, t), Lerp(from.blue, to.blue, t)};
}


enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

inline std::ostream& operator<< (std::ostream& out, const StrokeLineCap cap) {
    using namespace std::literals;
    switch (cap) {
    case StrokeLineCap::BUTT: out << "butt"sv; break;
    case StrokeLineCap::ROUND: out << "round"sv; break;
    case StrokeLineCap::SQUARE: out << "square"sv; break;
    default: break;
    }
    return out;
}

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

inline std::ostream& operator<< (std::ostream& out, const StrokeLineJoin join) {
    using namespace std::literals;
    switch (join) {
    case StrokeLineJoin::ARCS: out << "arcs"sv; break;
    case StrokeLineJoin::BEVEL: out << "bevel"sv; break;
    case StrokeLineJoin::MITER: out << "miter"sv; break;
    case StrokeLineJoin::MITER_CLIP: out << "miter-clip"sv; break;
    case StrokeLineJoin::ROUND: out << "round"sv; break;
    default: break;
    }
    return out;
}

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fillColor_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color) {
        strokeColor_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeWidth(double width) {
        strokeWidth_ = width;
        return AsOwner();
    }
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        strokeLineCap_ = line_cap;
        return AsOwner();
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        strokeLineJoin_ = line_join;
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fillColor_) {
            out << " fill=\""sv << *fillColor_ << "\""sv;
        }
        if (strokeColor_) {
            out << " stroke=\""sv << *strokeColor_ << "\""sv;
        }
        if (strokeWidth_) {
            out << " stroke-width=\""sv << *strokeWidth_ << "\""sv;
        }
        if (strokeLineCap_) {
            out << " stroke-linecap=\""sv << *strokeLineCap_ << "\""sv;
        }
        if (strokeLineJoin_) {
            out << " stroke-linejoin=\""sv << *strokeLineJoin_ << "\""sv;
        }
    }

private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner - наследник PathProps
        return static_cast<Owner&>(*this);
    }
    std::optional<Color> fillColor_;
    std::optional<Color> strokeColor_;
    std::optional<double> strokeWidth_;
    std::optional<StrokeLineCap> strokeLineCap_;
    std::optional<StrokeLineJoin> strokeLineJoin_;
};


/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final: public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point p);

private:
    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final: public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;

    Point position_ = {0,0};
    Point offset_ = {0,0};
    uint32_t fontSize_ = 1;
    std::string fontFamily_;
    std::string fontWeight_;
    std::string data_;
};


}  // namespace svg
