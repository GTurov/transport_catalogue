#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::render(const RenderContext& context) const {
    context.renderIndent();

    renderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::setCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::setRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::renderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle";
    out << " cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\""sv;
    out << " r=\""sv << radius_ << "\""sv;
    renderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ---------------

Polyline& Polyline::addPoint(Point p) {
    points_.push_back(p);
    return *this;
}

void Polyline::renderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool firstPoint = true;
    for (Point p: points_) {
        out << (firstPoint?""sv:" "sv) << p.x << ","sv << p.y;
        firstPoint = false;
    }
    out << "\""sv;
    renderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Text ------------------

Text& Text::setPosition(Point pos) {
    position_ = pos;
    return *this;
}

Text& Text::setOffset(Point offset) {
    offset_ =  offset;
    return *this;
}

Text& Text::setFontSize(uint32_t size) {
    fontSize_ = size;
    return *this;
}

Text& Text::setFontFamily(std::string fontFamily) {
    fontFamily_ = fontFamily;
    return *this;
}

Text& Text::setFontWeight(std::string fontWeight) {
    fontWeight_ = fontWeight;
    return *this;
}

Text& Text::setData(std::string data) {
    data_ = data;
    return *this;
}
void Text::renderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text";
    out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\""sv;
    out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
    out << " font-size=\""sv << fontSize_ << "\"";
    if (fontFamily_.size() != 0) {
        out << " font-family=\""sv << fontFamily_ << "\""sv;
    }
    if (fontWeight_.size() != 0 && fontWeight_ != "normal"sv) {
        out << " font-weight=\""sv << fontWeight_ << "\""sv;
    }
    renderAttrs(context.out);
    out << ">"sv;
    for (char c: data_) {
        switch (c) {
        case '"': out << "&quot;"sv ; break;
        case '\'': out << "&apos;"sv ; break;
        case '<': out << "&lt;"sv ; break;
        case '>': out << "&gt;"sv ; break;
        case '&': out << "&amp;"sv ; break;
        default:
            out << c;
        }
    }
    out << "</text>"sv;
}

// --------- Document -----------------

void Document::addPtr(std::unique_ptr<Object> &&obj) {
    objects_.push_back(std::move(obj));
}

void Document::render(std::ostream &out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
    RenderContext ctx(out, 2, 2);
    for (int i = 0; i < (int) objects_.size(); ++i) {
        objects_.at(i)->render(ctx);
        //out << "\n"sv;
    }
    out << "</svg>"sv;
    //out<< std::endl;
}

}  // namespace svg
