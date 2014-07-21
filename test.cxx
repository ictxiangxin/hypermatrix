#include <iostream>
#include "hm.h"

using namespace std;
using namespace hm;

int main()
{
    int x, y, v;
    matrix<int> *m1 = new matrix<int>(40000, 20000, 0);
    matrix<int> *m2 = new matrix<int>(1000, 1000, 0, true);
    matrix<int> *tm2;
    for(size_t i = 0; i < 1000000; i++)
    {
        cin >> x >> y >> v;
        m1->set(x, y, v);
        if(m1->get(x, y) != v)
            return -1;
    }
    for(size_t i = 0; i < 1000; i++)
        for(size_t j = 0; j <= i; j++)
            m2->set(i, j, i * 1000 + j);
    tm2 = m2->copy();
    for(size_t i = 0; i < 1000; i++)
        for(size_t j = 0; j <= i; j++)
        if(tm2->get(j, i) != i * 1000 + j)
            return -1;
}
