export module graphics:textures;

import :shapes;
import :gpu;

export class Texture {
public:
    Texture(Point _point, const char* _path):
        point{_point},
        path{_path} {}
    void print() const;
private:
    const char* path;
    Point point;
};