#ifndef SVGNESTCPP_H
#define SVGNESTCPP_H

#include <memory>
#include <iterator>

namespace binpack2d {

class RawShape;
using RawShapePtr = std::unique_ptr<RawShape>;

class Shape {
    RawShapePtr rshape_;
public:

    Shape();
    ~Shape();

    using Ptr = std::shared_ptr<Shape>;

protected:
    RawShape& rawShape() const { return *rshape_; }
};

class Rectangle: public Shape {
public:
    using Ptr = std::shared_ptr<Rectangle>;

};

class Ellipse: public Shape {
public:
    using Ptr = std::shared_ptr<Ellipse>;

};

class Polygon: public Shape {
public:
    using Ptr = std::shared_ptr<Polygon>;

};

class Item {
    Shape::Ptr shape_;

public:

    void translate();
    void rotate();

};

class Bin {
public:

};

class ItemRange;

class Packager {

    class Impl; std::unique_ptr<Impl> impl_;

    using ItemIndex = unsigned long;

    ItemIndex itemCount() const;

    Item& item(ItemIndex idx);

public:

    Packager();
    ~Packager();

    struct Config {
        unsigned long minObjectDistance;
    };

    void binShape(const Rectangle::Ptr& shape);
    void binShape(const Ellipse::Ptr& shape);

    void arrange(const ItemRange& range,
                 const Rectangle::Ptr& bin,
                 const Config& config = {0} );

    template<class...Args> void arrange(const ItemRange range,
                                        const Rectangle::Ptr& bin,
                                        Args...args)
    {
        arrange(range, bin, Config{args...});
    }

};

}

#endif // SVGNESTCPP_H
