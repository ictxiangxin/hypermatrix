#include <iostream>
#include "hypermatrix.h"

using namespace std;
using namespace hm;

int main()
{
    int x, y, v;
    matrix<int> *m = new matrix<int>(40000, 20000, 0);
    for(size_t i = 0; i < 1000000; i++)
    {
        cin >> x >> y >> v;
        m->set(x, y, v);
        if(m->get(x, y) != v)
            return -1;
    }
}
