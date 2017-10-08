# DetectionEvaluator.hpp
A class that helps to evaluate the sensitivity and precision of the results data of image detection algorithms.



## How to Mearsure
![Diagram of Sample Data](/assets/diagram_of_sample_data.svg?sanitize=true)
Note that the y-axis grows downward, as shown in the diagram above.

The white box is the correct answer (Ground Truth), and the red box is detected by your algorithm. The red box that intersects the rightmost white box intersects only a small part of the white box.

Evaluator calculates how much the intersection occupies in the GT, and if it exceeds the specified value, it is considered TP. Here, the specified value is called *Accept Rate*.

If several red boxes cross the same white box, the red box that occupies most of the white box is the TP.

Therefore, the result of the sample data is 2 TP, 3 FP, and 1 FN.



## How to Use
You just need to remember the `smns::Evaluator` class, and there is only one variable to care about, *Accept Rate*.



### Evaluator Definition
You can define `Evaluator` which *Accept Rate* is the default value(=0.2) using default constructor.
```cpp
Evaluator evaluator;
```

Or define by specifying.
```cpp
auto evaluator = Evaluator(0.4);
```

Or modifying later.
```cpp
evaluator.accept_rate(0.8);
```



### Enter the Data.
Then prepare the data to be evaluated. The type of data is `Evaluator::Items`, which is equal to `std::list<Item>`.
```cpp
Evaluator::Items GTs;
Evaluator::Items detected;
```

Because `Evaluator::Rectangle` is used internally, you must separate the data in bracket. Represents the x, y, width, and height values, respectively.
```cpp
// Add GT(abbr. of Ground Truth) data.
GTs.push_back({ 4, 2 , 12, 10 });
//              ^  ^    ^   ^
//              x  y    w   h
```

Or you can also use emplace_back. I prefer this method.
```cpp
GTs.emplace_back(14, 10, 7, 7);
//                ^   ^  ^  ^
//                x   y  w  h
```



### Evaluate
If you have all the data ready, you can use `Eavluator` as you would call a function. The parameter is GT and the detected data. The return type is `Evaluator::Result`, which is equal to `std::array<size_t, 3>`.
```cpp
auto rst = evaluator(GTs, detected);
```

`Evaluator::Result` has methods to perform calculation such as *PPV*, *TPR*, etc., and can output it directly to `std::ostream`.
```cpp
cout << rst << endl;
// TP:    2 | FP:    3 | FN:    1 | PPV: 0.4 | TPR: 0.666667
```

`Evaluator::Result` also provides an addition operation to easily accumulate the results of multiple evaluations.
```cpp
auto rst2 = Evaluator::Result(rst);

cout << rst + rst2 << endl;
// TP:    4 | FP:    6 | FN:    2 | PPV: 0.4 | TPR: 0.666667
```



---
The original code for the example above is [here](/src/main.cpp).