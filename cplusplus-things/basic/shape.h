/*
 * @Descripttion: 
 * @Author: HuSharp
 * @Date: 2022-01-30 18:17:36
 * @LastEditTime: 2022-01-30 20:17:09
 * @@Email: ihusharp@gmail.com
 */
enum class shape_type {
    circle,
    triangle,
    rectangle,
};

class shape { };
class circle : public shape {};
class triangle : public shape {};
class rectangle : public shape {};

shape *create_shape(shape_type type) {
    switch (type) {
    case shape_type::circle:
        return new circle();
    case shape_type::triangle:
        return new triangle();
    case shape_type::rectangle:
        return new rectangle();
    }
}