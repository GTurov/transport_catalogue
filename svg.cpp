#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle";
    out << " cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\""sv;
    out << " r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ---------------

Polyline& Polyline::AddPoint(Point p) {
    points_.push_back(p);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool firstPoint = true;
    for (Point p: points_) {
        out << (firstPoint?""sv:" "sv) << p.x << ","sv << p.y;
        firstPoint = false;
    }
    out << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos) {
    position_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ =  offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}
void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text";
    RenderAttrs(context.out);
    out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\""sv;
    out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
    out << " font-size=\""sv << font_size_ << "\"";
    if (font_family_.size() != 0) {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if (font_weight_.size() != 0 && font_weight_ != "normal"sv) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
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

void Document::AddPtr(std::unique_ptr<Object> &&obj) {
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream &out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
    RenderContext ctx(out, 2, 2);
    for (int i = 0; i < (int) objects_.size(); ++i) {
        objects_.at(i)->Render(ctx);
        //out << "\n"sv;
    }
    out << "</svg>"sv;
    //out<< std::endl;
}

}  // namespace svg
