#include <stdio.h>
#include <math.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
using namespace std;

class Bucket {
public:
    int size;
    
    Bucket(int size);
    ~Bucket();
    void add(void *item, double score);
    bool full();
    void transfer(void* out[]);

private:
    class BucketItem {
    public:
        double score;
        void *item;
    };

    int num_;
    BucketItem *const items_;
};

class Position {
public:
    double x;
    double y;

    Position(double _x, double _y): x (_x), y (_y) { }
};

class Quad {
    public:
        Quad(Quad *parent);
        ~Quad();
        int getNum() { return num_; }
        
        Position transform(uint8_t index, Position pos) {
            switch (index) {
            case 0:
                return Position(pos.x * 2.0, pos.y * 2.0);
            case 1:
                return Position(pos.x * 2.0, (pos.y - 0.5) * 2.0);
            case 2:
                return Position((pos.x - 0.5) * 2.0, pos.y * 2.0);
            case 3:
                return Position((pos.x - 0.5) * 2.0, (pos.y - 0.5) * 2.0);
            }
        }

        uint8_t indexFromPoint(Position pos) {
            if (pos.x < 0.5) {
                if (pos.y < 0.5) {
                    return 0;
                } else {
                    return 1;
                }
            } else {
                if (pos.y < 0.5) {
                    return 2;
                } else {
                    return 3;
                }
            }
        }

        int getAncestors();
        int guessLevel(int num);
        void add(void *item, Position pos);
        bool intersects(Position center, double rad1sq, double rad2sq);

        void collectItemsInBand(Position center, double rad1sq, double rad2sq, double scale, Bucket &bucket, bool skip_test);

        // a band is defined by a center point and an inner radius and outer radius
        void collectItemsInBand(Position center, double rad1, double rad2, Bucket &bucket) {
            return collectItemsInBand(center, rad1*rad1, rad2*rad2, 1.0, bucket, false);
        }
        
        void forAllItems(void (*func)(void *));
        void print(int indent);

    private:
        Quad *parent_;
        Quad *children_[4];
        void *item_;
        Position pos_;
        int num_;
};

/* QuadTree
 *
 * Spatial index that can efficiently return the n items closest to a point
 *
 */
class QuadTree {
    public:
        QuadTree(): topLeft_ (0, 0), bottomRight_ (0, 0), top_ (NULL) { }
        void init(int num, void* items[], Position (*getPos)(void *));
        void forAllItems(void (*func)(void *));
        void getItemsNear(int num, Position pos, void* out[]);
        void print() { top_.print(0); }

    private:
        Position transform(Position pos) {
            pos.x = (pos.x - topLeft_.x) / width_;
            pos.y = (pos.y - topLeft_.y) / width_;

            return pos;
        }
        Position topLeft_;
        Position bottomRight_;
        double width_;
        Quad top_;
};

