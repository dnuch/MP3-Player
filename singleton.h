//
// Created by drnuc on 9/17/2018.
//

#ifndef SINGLETON_H
#define SINGLETON_H

template <typename T>
class Singleton
{
public:
    Singleton(Singleton const &) = delete;
    Singleton& operator=(Singleton const &) = delete;

    static T & getInstance() {
        static T instance;
        return instance;
    }

protected:
    Singleton() = default;
    ~Singleton() = default;
};

#endif //SINGLETON_H
