#include "quadtree.h"


//////// Bucket ////////

Bucket::Bucket(int _size): 
    size(_size), 
    num_(0), 
    items_(new BucketItem[_size]) 
{
}

Bucket::~Bucket() {
    delete []items_;
}

void Bucket::add(void *item, double score) {
    int i;
    for (i = 0; i<num_; ++i) {
        if (items_[i].score < score) {
            break;
        }
    }

    if (i < size) {
        for (int j=size-1; j>i; --j) {
            items_[j] = items_[j-1];
        }

        items_[i].item = item;
        items_[i].score = score;

        if (num_ < size) {
            ++num_;
        }
    }
}

bool Bucket::full() {
    return num_ == size;
}

void Bucket::transfer(void* out[]) {
    for (int i=0; i<num_; ++i) {
        out[i] = items_[i].item;
    }
}


/////////// Quad //////////

Quad::Quad(Quad* parent): 
        parent_ (parent), 
        item_ (NULL),
        pos_ (0.0,0.0),
        num_ (0)
{
    for (int i=0; i<4; ++i) {
        children_[i] = NULL;
    }
}

Quad::~Quad() {
    for (int i=0; i<4; ++i) {
        if (children_[i] != NULL) {
            delete children_[i];
        }
    }
}

int Quad::getAncestors() {
    if (parent_ == NULL) {
        return 0;
    } else {
        return 1 + parent_->getAncestors();
    }
}

int Quad::guessLevel(int num) {
    if (parent_ == NULL || num_ >= num) {
        return getAncestors();
    } else {
        return parent_->guessLevel(num);
    }
}

void Quad::add(void *item, Position pos) {
    //XXX: what about items with the same position?
    if (*children_ == NULL) {
        if (item_ == NULL) {
            // accept this item and return
            item_ = item;
            pos_ = pos;
            num_ = 1;
            return;
        } else {
            // create children and pass our item down
            for (int i=0; i<4; i++) {
                children_[i] = new Quad(this);
            }
            num_ = 0;
            add(item_, pos_);
        }
    }
   
    uint8_t index = indexFromPoint(pos);
    children_[index]->add(item, transform(index, pos));
    
    ++num_;
}

// Does this quad intersect the given band
//
// center: center of the band
// rad1sq: inner radius squared
// rad2sq: outer radius squared
//
bool Quad::intersects(Position center, double rad1sq, double rad2sq) {
    int inside = 0;
    int inband = 0;
    int outside = 0;

    for (int x=0; x<=1; ++x) {
        for (int y=0; y<=1; ++y) {
            double a = center.x - x;
            double b = center.y - y;
            double d = a*a + b*b;
            if (d < rad1sq) {
                ++inside;
            } else if (d < rad2sq) {
                ++inband;
            } else {
                ++outside;
            }
        }
    }

    if (outside == 4) {
        // all corners are outside band
        int x = center.x;
        int y = center.y;

        bool hinside = x >= 0.0 && x <= 1.0;
        bool vinside = y >= 0.0 && y <= 1.0;

        bool hnear = (x > 0.0 || -x*x > -rad2sq) && (x < 1.0 || (x-1.0)*(x-1.0) < rad2sq);
        bool vnear = (y > 0.0 || -y*y > -rad2sq) && (y < 1.0 || (y-1.0)*(y-1.0) < rad2sq);

        // are we touching anyway?
        return (hinside && vnear) || (vinside && hnear);
    } else {
        // is any corner not inside?
        return inside < 4;
    }
}

void Quad::collectItemsInBand(Position center, double rad1sq, double rad2sq, double scale, Bucket &bucket, bool skip_test) {
    if (*children_ != NULL) {
        for (int i=0; i<4; ++i) {
            Quad *child = children_[i];
            if (child->num_ == 0)
                continue;

            Position c = transform(i, center);
            double r1 = rad1sq*2*2;
            double r2 = rad2sq*2*2;
            if (skip_test || child->intersects(c, r1, r2)) {
                child->collectItemsInBand(c, r1, r2, scale*0.5, bucket, (child->num_ <= bucket.size));
            }
        }
    } else if (item_ != NULL) {
        //cout << "center: " << center.x << "," << center.y << "; pos: " << pos_.x << "," << pos_.y << endl;

        double a = center.x - pos_.x;
        double b = center.y - pos_.y;
        double d = a*a + b*b;

        //cout << "? " << rad1sq << " <= " << d << " <= " << rad2sq << endl;

        if (rad1sq <= d && d <= rad2sq) {
            bucket.add(item_, -d * scale * scale);
        }
    }
}

void Quad::forAllItems(void (*func)(void *)) {
    if (*children_ != NULL) {
        for (int i=0; i<4; ++i) {
            Quad *child = children_[i];
            if (child->num_ == 0)
                continue;

            child->forAllItems(func);
        }
    } else if (item_ != NULL) {
        func(item_);
    }
}

inline void print_indent(int indent) {
    for (int i=0; i<indent; ++i) {
        cout << "  ";
    }
}
void Quad::print(int indent) {
    if (*children_ != NULL) {
        print_indent(indent);
        cout << "-{" << endl;

        for (int i=0; i<4; ++i) {
            Quad *child = children_[i];

            child->print(indent + 1);
        }
    } else if (item_ != NULL) {
        print_indent(indent);
        cout << "-" << pos_.x << "," << pos_.y << endl;
    } else {
        print_indent(indent);
        cout << "-None" << endl;
    }
}


////////// QuadTree /////////

void QuadTree::init(int num, void* items[], Position (*getPos)(void *)) {
    topLeft_ = bottomRight_ = getPos(items[0]);

    for (int i=1; i<num; ++i) {
        Position pos = getPos(items[i]);
        if (pos.x < topLeft_.x)
            topLeft_.x = pos.x;
        if (pos.x > bottomRight_.x)
            bottomRight_.x = pos.x;
        if (pos.y < topLeft_.y)
            topLeft_.y = pos.y;
        if (pos.y > bottomRight_.y)
            bottomRight_.y = pos.y;
    }

    double width = bottomRight_.x - topLeft_.x;
    double height = bottomRight_.y - topLeft_.y;
    if (width > height) {
        if (width == 0.0) {
            width = 1.0;
        }
        bottomRight_.y += width - height;
        width_ = width;
    } else {
        if (height == 0.0) {
            height = 1.0;
        }
        bottomRight_.x += height - width;
        width_ = height;
    }

    for (int i=0; i<num; ++i) {
        top_.add(items[i], transform(getPos(items[i])));
    }
}

void QuadTree::forAllItems(void (*func)(void *)) {
    top_.forAllItems(func);
}

const double PI = 3.141592;
void QuadTree::getItemsNear(int num, Position pos, void* out[]) {
    if (top_.getNum() < num) {
        cout << "Wait! num=" << top_.getNum();
        exit(1);
    }

    Bucket bucket(num);

    pos = transform(pos);
    // guess first band to search, 
    // assuming items are uniformly distributed over space
    double rad1 = 0;
    double rad2 = sqrt(double(num) / top_.getNum() / PI);

    do {
        top_.collectItemsInBand(pos, rad1, rad2, bucket);

        rad1 = rad2;
        rad2 = rad2 * 2;

        if (rad2 == numeric_limits<double>::infinity()) {
            cout << "INFINITY";
            exit(1);
        }
    } while (!bucket.full());

    bucket.transfer(out);
}
