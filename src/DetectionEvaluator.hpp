#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <iostream>
#include <iomanip>
#include <list>
#include <array>

namespace smns {
    constexpr size_t RESULT_SIZE = 3;

    class Evaluator {
    private:
        class Point;
        class Rectangle;
        class Item;
    public:
        using Items = std::list<Item>;

        enum RESULT_DOMAIN { TP, FP, FN };

        class Result : public std::array<size_t, RESULT_SIZE> {
        private: using Base = std::array<size_t, RESULT_SIZE>;
        public:
            Result() :Base({ 0 }) {}

            double ppv() const {
                const auto& self = *this;
                return static_cast<double>(self[TP])
                    / (self[TP] + self[FP]);
            }

            double tpr() const {
                const auto& self = *this;
                return static_cast<double>(self[TP])
                    / (self[TP] + self[FN]);
            }

            double fnr() const { return 1 - tpr(); }
            double fdr() const { return 1 - ppv(); }

            double precision()      const { return ppv(); }
            double hit_rate()       const { return tpr(); }
            double recall()         const { return tpr(); }
            double sensitivity()    const { return tpr(); }

            double F_measure() const {
                const auto ppv = this->ppv();
                const auto tpr = this->tpr();
                return 2 * (ppv * tpr) / (ppv + tpr);
            }

            Result operator + (const Result& other) const {
                Result rst;
                for (auto i = 0; i < RESULT_SIZE; ++i)
                    rst[i] = (*this)[i] + other[i];
                return rst;
            }
        };

        Evaluator(double accept_rate) : _th_accept_rate(accept_rate) {}
        Evaluator() : Evaluator(DEFAULT_ACCEPT_RATE) {}

        Result evaluate(Items& GTs, Items& detected) {
            Result rst;

            // Initiate: Connect between GT and detected
            for (auto& GT : GTs) {
                for (auto& sbj : detected) {
                    if (!GT.self.isIntersect(sbj.self)) continue;
                    GT.connect(sbj);
                }
            }

            // Accept TPs
            for (auto& GT : GTs) {
                const double size_of_GT = static_cast<double>(GT.self.size());
                Item* accepted = nullptr;   // It may be TP
                double evaluated = 0;       // Size of Intersected Rectangle of accepted
                Items dropped;              // Set of dropped candidate

                for (auto& candidate : *GT.danglers) {
                    const double size_of_candidate = candidate.self.size();
                    const double size_of_intersect = GT.self.intersect(candidate.self).size();
                    const double intersect_rate = size_of_intersect /
                        (size_of_candidate - size_of_intersect + size_of_GT);

                    // Dose candidate satisfies the condition
                    if (intersect_rate < _th_accept_rate) {
                        dropped.push_back(candidate);
                        continue;
                    }

                    // Competition with pre-accepted
                    if (intersect_rate < evaluated) {
                        dropped.push_back(candidate);
                    }
                    // if pre-accepted defeated
                    else {
                        if (accepted)
                            dropped.push_back(*accepted);
                        accepted = &candidate;
                        evaluated = intersect_rate;
                    }
                }

                // Disconnect dropped candidates
                for (auto& candidate : dropped) {
                    GT.disconnect(candidate);
                }
            }

            // Choose a side
            for (auto& sbj : detected) {
                // it has a side or any side
                if (sbj.danglers->size() < 2)
                    continue;

                Items dropped;
                Item* accepted = nullptr;
                size_t evaluated = 0;

                for (auto& GT : *sbj.danglers) {
                    const size_t intersect_size = GT.self.intersect(sbj.self).size();

                    if (intersect_size < evaluated) {
                        accepted = &GT;
                        evaluated = intersect_size;
                    }
                    else {
                        if (accepted)
                            dropped.push_back(*accepted);
                        accepted = &GT;
                        evaluated = intersect_size;
                    }
                }

                for (auto& GT : dropped) {
                    sbj.disconnect(GT);
                }
            }

            // Count
            for (auto& sbj : detected) {
                if (sbj.danglers->size() == 0) {
                    rst[FP]++;
                }
                else {
                    assert(sbj.danglers->size() == 1);
                    rst[TP]++;
                }
            }

            for (auto& GT : GTs) {
                if (GT.danglers->size() != 0) continue;
                rst[FN]++;
            }

            return rst;
        }

        Result operator () (Items& GTs, Items& detected) {
            return this->evaluate(GTs, detected);
        }

        void accept_rate(double val) { _th_accept_rate = val; }
        double accept_rate() { return _th_accept_rate; }

        static double DEFAULT_ACCEPT_RATE;

    private:
        double _th_accept_rate;

        class Point {
        public:
            Point(size_t x, size_t y) : x(x), y(y) {}
            Point(const Point& other) : Point(other.x, other.y) {}
            Point() :Point(0, 0) {}

            size_t x;
            size_t y;

            Point& operator = (const Point& other) {
                x = other.x;
                y = other.y;
                return *this;
            }

            bool operator == (const Point& other) const {
                return (this->x == other.x &&
                        this->y == other.y);
            }

            bool operator != (const Point& other) const {
                return !(this->operator==(other));
            }

            bool operator < (const Point& other) const {
                if (this->operator==(other)) return false;
                return (this->x < other.x &&
                        this->y < other.y);
            }

            bool operator > (const Point& other) const {
                if (this->operator==(other)) return false;
                return (this->x > other.x &&
                        this->y > other.y);
            }
        };
        class Rectangle {
        public:
            Rectangle(const Point& location,
                      const size_t width,
                      const size_t height) :
                location(location),
                width(width),
                height(height) {}
            Rectangle(const Rectangle& other) :
                Rectangle(other.location,
                          other.width,
                          other.height) {}
            Rectangle(const size_t x,
                      const size_t y,
                      const size_t width,
                      const size_t height) :
                Rectangle({ x, y }, width, height) {}
            Rectangle() : Rectangle({ 0, 0 }, 0, 0) {}

            Point location;
            size_t width;
            size_t height;

            size_t top()    const { return location.y; }
            size_t right()  const { return location.x + width; }
            size_t bottom() const { return location.y + height; }
            size_t left()   const { return location.x; }
            size_t size()   const { return width * height; }

            bool isEmpty() const {
                static const Rectangle empty;
                return *this == empty;
            }

            bool isIntersect(const Rectangle& rectangle) const {
                return isIntersect(*this, rectangle);
            }

            Rectangle intersect(const Rectangle& rectangle) const {
                return intersect(*this, rectangle);
            }

            static bool isIntersect(const Rectangle& lh,
                                    const Rectangle& rh) {
                if (lh.right() < rh.left() || rh.right() < lh.left())
                    return false;
                if (lh.bottom() < rh.top() || rh.bottom() < lh.top())
                    return false;
                return true;
            }

            static Rectangle intersect(const Rectangle& lh,
                                       const Rectangle& rh) {
                Rectangle rst;
                if (!isIntersect(lh, rh)) return rst;

                using std::max;
                using std::min;

                rst.location.x = max(lh.left(), rh.left());
                rst.location.y = max(lh.top(), rh.top());
                rst.width = min(lh.right(), rh.right()) - rst.location.x;
                rst.height = min(lh.bottom(), rh.bottom()) - rst.location.y;

                return rst;
            }

            bool operator == (const Rectangle& other) const {
                return (location == other.location &&
                        width == other.width &&
                        height == other.height);
            }

            bool operator != (const Rectangle& other) const {
                return !(this->operator==(other));
            }

            Rectangle& operator = (const Rectangle& other) {
                location = other.location;
                width = other.width;
                height = other.height;
                return *this;
            }
        };
        class Item {
        public:
            using Items = std::list <Item>;

            Item(const Item& other) : self(other.self) {
                danglers = other.danglers;
            }
            Item(const Rectangle& self) : self(self) {
                danglers = std::make_shared<Items>();
            }
            Item(const Point& location,
                 const size_t width,
                 const size_t height) :
                Item(Rectangle(location, width, height)) {}
            Item(const size_t x,
                 const size_t y,
                 const size_t width,
                 const size_t height) :
                Item({ x, y }, width, height) {}

            const Rectangle self;
            std::shared_ptr<Items> danglers;

            void connect(Item& other) {
                connect(*this, other);
            }

            void disconnect(Item& other) {
                disconnect(*this, other);
            }

            static void connect(Item& lh, Item& rh) {
                lh.danglers->push_back(rh);
                rh.danglers->push_back(lh);
            }

            static void disconnect(Item& lh, Item& rh) {
                remove_one(*lh.danglers, rh);
                remove_one(*rh.danglers, lh);
            }

            Item& operator = (const Item&) = delete;

            bool operator == (const Item& other) const {
                return self == other.self;
            }

            bool operator != (const Item& other) const {
                return !(this->operator==(other));
            }

        private:
            static void remove_one(Items& src, const Item& trg) {
                Items::const_iterator& it = src.begin();
                auto& end = src.end();

                for (; it != end; ++it) {
                    if (*it != trg) continue;

                    src.erase(it);
                    break;
                }
            }
        };
    };


    double Evaluator::DEFAULT_ACCEPT_RATE = 0.2;


    std::ostream &operator<<(std::ostream& os, const Evaluator::Result& m) {
        using namespace std;
        constexpr size_t W_SIZE = 5;
        constexpr char separator[] = " | ";
        return os
            << "TP:" << setw(W_SIZE) << m[0] << separator
            << "FP:" << setw(W_SIZE) << m[1] << separator
            << "FN:" << setw(W_SIZE) << m[2] << separator
            << "PPV: " << m.ppv() << separator
            << "TPR: " << m.tpr();
    }
}