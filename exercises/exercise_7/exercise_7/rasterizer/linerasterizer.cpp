#include "linerasterizer.h"


/*
 * \class LineRasterizer
 * A class which scanconverts a straight line. It computes the pixels such that they are as close to the
 * the ideal line as possible.
 */

/*
 * Parameterized constructor creates an instance of a line rasterizer
 * \param x1 - the x-coordinate of the first vertex
 * \param y1 - the y-coordinate of the first vertex
 * \param x2 - the x-coordinate of the second vertex
 * \param y2 - the y-coordinate of the second vertex
 */
LineRasterizer::LineRasterizer(int x1, int y1, int x2, int y2)
{
    this->initialize_line(x1, y1, x2, y2);
}

/*
 * Destroys the current instance of the line rasterizer
 */
LineRasterizer::~LineRasterizer()
{}

/*
 * Initializes the LineRasterizer with a new line
 * \param x1 - The x-coordinate of the first vertex
 * \param y1 - The y-coorsinate of the first vertex
 * \param x2 - The x-coordinate of the second vertex
 * \param y2 - The y-coordinate of the second vertex
 */
void LineRasterizer::init(int x1, int y1, int x2, int y2)
{
    this->initialize_line(x1, y1, x2, y2);
}

/*
 * Checks if there are fragments/pixels of the line ready for use
 * \return true if there are more fragments of the line, else false is returned
 */
bool LineRasterizer::more_fragments() const
{
    return this->valid;
}

/*
 * Computes the next fragment of the line
 */
void LineRasterizer::next_fragment()
{
    // Run the innerloop once
    // Dereference the pointer to the private member function
    // It looks strange; but this is the way it is done!
    (this->*innerloop)();
}

/*
 * Returns a vector which contains all the pixels of the line
 */
std::vector<glm::ivec2> LineRasterizer::all_pixels()
{
    std::vector<glm::ivec2> points;

    while (this->more_fragments()) {
        points.push_back(glm::ivec2(x_current, y_current));
        this->next_fragment();
    }
    return points;
}


/*
 * Returns the current x-coordinate of the current fragment/pixel of the line
 * It is only valid to call this function if "more_fragments()" returns true,
 * else a "runtime_error" exception is thrown
 * \return The x-coordinate of the current line fragment/pixel
 */
int LineRasterizer::x() const
{
    if (!this->valid) {
        throw std::runtime_error("LineRasterizer::x(): Invalid State");
    }
    return this->x_current;
}

/*
 * Returns the current y-coordinate of the current fragment/pixel of the line
 * It is only valid to call this function if "more_fragments()" returns true,
 * else a "runtime_error" exception is thrown
 * \return The y-coordinate of the current line fragment/pixel
 */
int LineRasterizer::y() const
{
    if (!this->valid) {
        throw std::runtime_error("LineRasterizer::y(): Invalid State");
    }
    return this->y_current;
}

/*
 * Protected functions
 */

/*
 * Private functions
 */

/*
 * Initializes the LineRasterizer with the two vertices
 */
void LineRasterizer::initialize_line(int x1, int y1, int x2, int y2)
{
    this->x_start = x1;
    this->y_start = y1;

    this->x_stop = x2;
    this->y_stop = y2;

    this->x_current = this->x_start;
    this->y_current = this->y_start;

    this->dx = this->x_stop - this->x_start;
    this->dy = this->y_stop - this->y_start;

    this->abs_2dx = std::abs(this->dx) << 1; // 2 * |dx|
    this->abs_2dy = std::abs(this->dy) << 1; // 2 * |dy|

    this->x_step = (this->dx < 0) ? -1 : 1;
    this->y_step = (this->dy < 0) ? -1 : 1;

    if (this->abs_2dx > this->abs_2dy) {
        // the line is x-dominant
        this->left_right = (this->x_step > 0);
        this->d = this->abs_2dy - (this->abs_2dx >> 1);
        this->valid = (this->x_start != this->x_stop);
        this->innerloop = &LineRasterizer::x_dominant_innerloop;
    }
    else {
        // the line is y-dominant
        this->left_right = (this->y_step > 0);
        this->d = this->abs_2dx - (this->abs_2dy >> 1);
        this->valid = (this->y_start != this->y_stop);
        this->innerloop = &LineRasterizer::y_dominant_innerloop;
    }
}

/*
 * Runs the x-dominant innerloop
 */
void LineRasterizer::x_dominant_innerloop()
{
    if ((this->valid = (this->x_current != this->x_stop))) {
        if (this->d > 0 || (this->d == 0 && this->left_right)) {
            this->y_current += this->y_step;
            this->d         -= this->abs_2dx;
        }
        this->x_current += this->x_step;
        this->d         += this->abs_2dy;
    }
}

/*
 * Runs the y-dominant innerloop
 */
void LineRasterizer::y_dominant_innerloop()
{
    if ((this->valid = (this->y_current != this->y_stop))) {
        if (this->d > 0 || (this->d == 0 && this->left_right)) {
            this->x_current += this->x_step;
            this->d         -= this->abs_2dy;
        }
        this->y_current += this->y_step;
        this->d         += this->abs_2dx;
    }
}
