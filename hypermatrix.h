#ifndef _HYPERMATRIX_H_
#define _HYPERMATRIX_H_
#ifdef DEBUG
#include <stdio.h>
#define debug(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define debug(fmt, ...) ((void *)0)
#endif

#define STATUS_NORMAL 1
#define STATUS_SPARSE 2


namespace hm
{
    inline void hm_swap(size_t &a, size_t &b)
    {
        size_t tmp = a;
        a = b;
        b = tmp;
    }

    template<typename VALUE_T>
    class sparse
    {
        public:
            size_t y;
            VALUE_T v;
            size_t next;

            void operator=(VALUE_T v)
            {
                this->v = v;
            }
            
            void operator=(sparse &sp)
            {
                this->y = sp.y;
                this->v = sp.v;
                this->next = sp.next;
            }
    };

    template<typename VALUE_T>
    static void init_sparse(sparse<VALUE_T> &sp, VALUE_T v)
    {
        sp.y = 0;
        sp.v = v;
        sp.next = -1;
    }

    template<typename VALUE_T>
    class sparselist
    {
        public:
            size_t x;
            sparse<VALUE_T> *sl; // sparse list
            size_t size;
            size_t alloc_size;
            VALUE_T v;

            sparselist()
            {
                this->x = 0;
                this->size = 0;
                this->alloc_size = 0;
                this->sl = NULL;
            }

            ~sparselist()
            {
                delete [] this->sl;
            }

            void init(size_t size, size_t alloc_size, VALUE_T v, size_t x)
            {
                this->x = x;
                this->size = size;
                this->alloc_size = alloc_size;
                this->sl = new sparse<VALUE_T>[this->size];
                for(size_t i = 0; i < this->size; i++)
                    init_sparse(this->sl[i], v);
            }

            VALUE_T get_value(size_t index)
            {
                size_t p = 0;
                if(this->sl[p].next != -1)
                {
                    do
                    {
                        if(this->sl[p].y == index)
                            return this->sl[p].v;
                        else
                            p = this->sl[p].next;
                    } while(p != 0 && this->sl[p].y <= index);
                }
                return this->v;
            }

            // true  - value type change
            // false - no value type change
            bool set_value(size_t index, VALUE_T v)
            {
                size_t p = 0;
                size_t end = p;
                if(this->sl[p].next != -1)
                {
                    do
                    {
                        if(this->sl[p].y == index)
                        {
                            if(this->v == v)
                            {
                                size_t next = this->sl[p].next;
                                if(p == 0 && next != 0)
                                {
                                    this->sl[0] = this->sl[next];
                                    init_sparse(this->sl[next], this->v);
                                }
                                else
                                {
                                    this->sl[end].next = next;
                                    init_sparse(this->sl[p], this->v);
                                }
                                return true;
                            }
                            else
                            {
                                this->sl[p].v = v;
                                return false;
                            }
                        }
                        else
                        {
                            end = p;
                            p = this->sl[p].next;
                        }
                    } while(p != 0 && this->sl[p].y <= index);
                }
                if(this->v == v)
                    return false;
                for(size_t i = 0; i < this->size; i++)
                    if(this->sl[i].next == -1)
                    {
                        if(end == 0 && this->sl[end].y > index)
                        {
                            this->sl[i] = this->sl[end];
                            this->sl[0].next = i;
                            this->sl[0].y = index;
                            this->sl[0].v = v;
                        }
                        else
                        {
                            this->sl[i].next = this->sl[end].next == -1 ? 0 : this->sl[end].next;
                            this->sl[i].y = index;
                            this->sl[i].v = v;
                            this->sl[end].next = i;
                        }
                        return true;
                    }
                sparse<VALUE_T> *new_sl = new sparse<VALUE_T>[this->size + this->alloc_size];
                for(size_t i = 0; i < this->size; i++)
                    new_sl[i] = this->sl[i];
                for(size_t i = this->size; i < this->size + this->alloc_size; i++)
                    init_sparse(new_sl[i], this->v);
                size_t new_i = this->size;
                this->size += this->alloc_size;
                delete [] this->sl;
                this->sl = new_sl;
                new_sl[new_i].y = index;
                new_sl[new_i].v = v;
                new_sl[new_i].next = new_sl[end].next;
                new_sl[end].next = new_i;
                return true;
            }

            void operator=(sparselist &spl)
            {
                if(this->size < spl.size)
                {
                    delete [] this->sl;
                    this->sl = new sparse<VALUE_T>[spl.size];
                }
                this->x = spl.x;
                this->size = spl.size;
                this->alloc_size = spl.alloc_size;
                for(size_t i = 0; i < spl.size; i++)
                    this->sl[i] = spl.sl[i];
            }
    };

    template<typename VALUE_T>
    class sparseblock
    {
        private:
            sparselist<VALUE_T> *sb;
            size_t size;
            VALUE_T v;
            bool transpose;
            bool symmetric;

        public:
            sparseblock(size_t block_size, size_t size, size_t alloc_size, VALUE_T v, bool transpose, bool symmetric = false)
            {
                this->size = block_size;
                this->v = v;
                this->transpose = transpose;
                this->symmetric = symmetric;
                this->sb = new sparselist<VALUE_T>[this->size];
                if(this->symmetric)
                {
                    for(size_t i = 0; i < this->size; i++)
                        this->sb[i].init(i < size ? i + 1 : size, i < alloc_size ? i + 1 : alloc_size, this->v, i);
                }
                else
                {
                    for(size_t i = 0; i < this->size; i++)
                        this->sb[i].init(size, alloc_size, this->v, i);
                }
            }

            ~sparseblock()
            {
                delete [] this->sb;
            }

            VALUE_T get_value(size_t index, size_t jndex)
            {
                if(this->transpose)
                    hm_swap(index, jndex);
                return this->sb[index].get_value(jndex);
            }

            bool set_value(size_t index, size_t jndex, VALUE_T v)
            {
                if(this->transpose)
                    hm_swap(index, jndex);
                return this->sb[index].set_value(jndex, v);
            }

            void fill_normal(VALUE_T *array, size_t m)
            {
                for(size_t i = 0; i < this->size; i++)
                {
                    size_t p = 0;
                    if(this->sb[i].sl[p].next != -1)
                    {
                        do
                        {
                            size_t offset;
                            if(this->symmetric)
                                offset = ((this->sb[i].x * (this->sb[i].x + 1)) >> 1) + this->sb[i].sl[p].y;
                            else
                                offset = this->sb[i].x * m + this->sb[i].sl[p].y;
                            array[offset] = this->sb[i].sl[p].v;
                            p = this->sb[i].sl[p].next;
                        } while(p != 0);
                    }
                }
            }
    };

    template<typename VALUE_T>
    class matrix
    {
        private:
            // 1 - normal matrix
            // 2 - sparse matrix
            int status;
            // matrix size: n * m
            size_t n;
            size_t m;
            size_t size;
            bool symmetric;
            sparseblock<VALUE_T> *_sparse;
            VALUE_T *_normal;
            VALUE_T v; // default value
            size_t c; // default value count
        public:
            matrix(size_t n, size_t m, VALUE_T v, bool symmetric);
            ~matrix();
            VALUE_T get(size_t index, size_t jndex);
            void set(size_t index, size_t jndex, VALUE_T v);
    };

    template<typename VALUE_T>
    matrix<VALUE_T>::matrix(size_t n, size_t m, VALUE_T v, bool symmetric = false)
    {
        this->n = n;
        this->m = m;
        this->v = v;
        this->symmetric = symmetric;
        this->c = 0;
        this->size = n * m;
        this->status = STATUS_SPARSE;
        this->size = n * m;
        if(this->symmetric)
        {
            if(this->m != this->n)
                throw 1;
        }
        size_t n_size = n >> 4;
        size_t m_size = m >> 4;
        n_size = n_size ? n_size : 1;
        m_size = m_size ? m_size : 1;
        if(this->n <= this->m)
            this->_sparse = new sparseblock<VALUE_T>(this->n, m_size, m_size, this->v, false, this->symmetric);
        else
            this->_sparse = new sparseblock<VALUE_T>(this->m, n_size, n_size, this->v, true, this->symmetric);
        this->_normal = 0;
    }

    template<typename VALUE_T>
    VALUE_T matrix<VALUE_T>::get(size_t index, size_t jndex)
    {
        if(index >= this->n || jndex >= this->m)
        {
            debug("Index out of range: (%d, %d)\n", index, jndex);
            return this->v;
        }
        if(this->symmetric)
            if(index < jndex)
                hm_swap(index, jndex);
        if(status == STATUS_SPARSE)
        {
            return this->_sparse->get_value(index, jndex);
        }
        if(status == STATUS_NORMAL)
        {
            if(this->symmetric)
                return this->_normal[((index * (index + 1)) >> 1) + jndex];
            else
                return this->_normal[index * this->m + jndex];
        }
    }

    template<typename VALUE_T>
    void matrix<VALUE_T>::set(size_t index, size_t jndex, VALUE_T v)
    {
        if(index >= this->n || jndex >= this->m)
        {
            debug("Index out of range: (%d, %d)\n", index, jndex);
            return;
        }
        if(this->symmetric)
            if(index < jndex)
                hm_swap(index, jndex);
        if(this->status == STATUS_SPARSE)
        {
            if(this->_sparse->set_value(index, jndex, v))
            {
                if(this->v == v)
                    this->c--;
                else
                    this->c++;
            }
            if(this->c > this->size >> 2)
            {
                debug("Matrix too dense, tranform to normal matrix.\n");
                if(this->symmetric)
                {
                    this->_normal = new VALUE_T[(this->n * (this->n + 1)) >> 1];
                    for(size_t i = 0; i < this->n; i++)
                        for(size_t j = 0; j <= i; j++)
                            this->_normal[((i * (i + 1)) >> 1) + j] = this->v;
                }
                else
                {
                    this->_normal = new VALUE_T[n * m];
                    for(size_t i = 0; i < this->n; i++)
                        for(size_t j = 0; j < this->m; j++)
                            this->_normal[i * this->m + j] = this->v;
                }
                this->_sparse->fill_normal(this->_normal, this->m);
                delete this->_sparse;
                this->_sparse = 0;
                this->status = STATUS_NORMAL;
                return;
            }
        }
        if(this->status == STATUS_NORMAL)
        {
            size_t offset;
            if(this->symmetric)
                offset = ((index * (index + 1)) >> 1) + jndex;
            else
                offset = index * this->m + jndex;
            if(this->_normal[offset] == this->v)
            {
                if(v != this->v)
                    this->c++;
            }
            else
            {
                if(v == this->v)
                    this->c--;
            }
            this->_normal[offset] = v;
            if(this->c < this->size >> 4)
            {
                debug("Matrix too sparse, tranform to sparse matrix.\n");
                size_t n_size = this->n >> 2;
                size_t m_size = this->m >> 2;
                n_size = n_size ? n_size : 1;
                m_size = m_size ? m_size : 1;
                if(this->n <= this->m)
                    this->_sparse = new sparseblock<VALUE_T>(this->n, m_size, m_size, this->v, false, this->symmetric);
                else
                    this->_sparse = new sparseblock<VALUE_T>(this->m, n_size, n_size, this->v, true, this->symmetric);
                if(this->symmetric)
                {
                    for(size_t i = 0; i < this->n; i++)
                        for(size_t j = 0; j <= i; j++)
                        {
                            size_t offset = ((i* (i+ 1)) >> 1) + j;
                            if(this->_normal[offset] != this->v)
                                this->_sparse->set_value(i, j, this->_normal[offset]);
                        }
                }
                else
                {
                    for(size_t i = 0; i < this->n; i++)
                        for(size_t j = 0; j < this->m; j++)
                        {
                            size_t offset = i * this->m + j;
                            if(this->_normal[offset] != this->v)
                                this->_sparse->set_value(i, j, this->_normal[offset]);
                        }
                }
                delete [] this->_normal;
                this->_normal = 0;
                this->status = STATUS_SPARSE;
                return;
            }
        }
    }
}

#endif
