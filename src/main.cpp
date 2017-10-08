#include <iostream>
#include "Evalumator.hpp"

int main() {
    using namespace std;
    using namespace smns;

    // Create Evaluator which Accept Rate is the default value(=0.2).
    Evaluator evaluator;

    // Or create by specifying.
     //Evaluator evaluator(0.4);
    //
    // Or modifying later.
    // evaluator.accept_rate(0.8);


    // Evaluator::Items = std::list<Evaluator::Item>
    // Evaluator::Item corresponds to a Evaluator::Rectangle
    // Evaluator::Rectangle is defined as a two-dimensional coordinate
    //  with the y-axis going downward and horizontally and vertically.
    // Evaluator::Item and Evaluator::Rectangle is hidden(private) class.
    Evaluator::Items GTs;
    Evaluator::Items detected;

    // Add GT(abbr of Ground Truth) data.
    GTs.push_back({ 4, 2, 12, 10 });
    //              ^  ^    ^   ^
    //              x  y    w   h

    // You can also use emplace_back.
    GTs.emplace_back(14, 10, 7, 7);
    //                ^   ^  ^  ^
    //                x   y  w  h

    GTs.emplace_back(32, 8, 7, 7);

    detected.emplace_back(8, 5, 9, 8);
    detected.emplace_back(14, 3, 4, 4);
    detected.emplace_back(14, 11, 9, 8);
    detected.emplace_back(20, 7, 9, 8);
    detected.emplace_back(27, 4, 6, 5);


    auto rst = evaluator(GTs, detected);
    cout << rst << endl;
    // TP:    2 | FP:    3 | FN:    1 | PPV: 0.4 | TPR: 0.666667

    auto rst2 = Evaluator::Result(rst);
    // Evaluator::Result provides an addition operation
    //  that adds each evaluation value.
    cout << rst + rst2 << endl;
    // TP:    4 | FP:    6 | FN:    2 | PPV: 0.4 | TPR: 0.666667
    
    return 0;
}