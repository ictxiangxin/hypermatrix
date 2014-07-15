
#define STATUS_NORMAL 1
#define STATUS_SPARSE 2

namespace hm
{
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
        sp.next = 0;
    }

    template<typename VALUE_T>
    class sparselist
    {
        public:
            size_t x;
            sparse<VALUE_T> *sl; // sparse list
            size_t size;
            size_t alloc_size;
            size_t next;
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

            void init(size_t size, size_t alloc_size, VALUE_T v)
            {
                this->x = 0;
                this->size = size;
                this->alloc_size = alloc_size;
                this->sl = new sparse<VALUE_T>[this->size];
                for(size_t i = 0; i < this->size; i++)
                    init_sparse(this->sl[i], v);
            }

            VALUE_T get_value(size_t index)
            {
                size_t p = 0;
                do
                {
                    if(this->sl[p].y == index)
                        return this->sl[p].v;
                    else
                        p = this->sl[p].next;
                } while(p != 0);
                return this->v;
            }

            // true  - value type change
            // false - no value type change
            bool set_value(size_t index, VALUE_T v)
            {
                size_t p = 0;
                size_t end = 0;
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
                } while(p != 0);
                if(this->v == v)
                    return false;
                for(size_t i = 0; i < this->size; i++)
                    if(this->sl[i].next == 0)
                    {
                        this->sl[i].y = index;
                        this->sl[i].v = v;
                        this->sl[end].next = i;
                        return true;
                    }
                sparse<VALUE_T> *new_sl = new sparse<VALUE_T>[this->size + this->alloc_size];
                for(size_t i = 0; i < this->size; i++)
                    new_sl[i] = this->sl[i];
                for(size_t i = this->size; i < this->size + this->alloc_size; i++)
                    init_sparse(new_sl[i], this->v);
                size_t new_i = this->size + 1;
                this->size += this->alloc_size;
                delete [] this->sl;
                this->sl = new_sl;
                new_sl[new_i].y = index;
                new_sl[new_i].v = v;
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
                this->next = next;
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
            size_t alloc_size;
            VALUE_T v;

        public:
            sparseblock(size_t size, size_t alloc_size, VALUE_T v)
            {
                this->size = size;
                this->alloc_size = alloc_size;
                this->v = v;
                this->sb = new sparselist<VALUE_T>[this->size];
                for(size_t i = 0; i < this->size; i++)
                    this->sb[i].init(this->size, this->alloc_size, this->v);
            }

            ~sparseblock()
            {
                delete [] this->sb;
            }

            VALUE_T get_value(size_t index, size_t jndex)
            {
                size_t p = 0;
                do
                {
                    if(this->sb[p].x == index)
                        return this->sb[p].get_value(jndex);
                    else
                        p = this->sb[p].next;
                } while(p != 0);
                return this->v;
            }

            bool set_value(size_t index, size_t jndex, VALUE_T v)
            {
                size_t p = 0;
                size_t end;
                do
                {
                    if(this->sb[p].x == index)
                        return this->sb[p].set_value(jndex, v);
                    else
                    {
                        end = p;
                        p = this->sb[p].next;
                    }
                } while(p != 0);
                if(this->v == v)
                    return false;
                for(size_t i = 0; i < this->size; i++)
                    if(this->sb[i].next == 0)
                    {
                        this->sb[i].x = index;
                        this->sb[i].set_value(jndex, v);
                        this->sb[end].next = i;
                        return true;
                    }
                sparselist<VALUE_T> *new_sb = new sparselist<VALUE_T>[this->size + this->alloc_size];
                for(size_t i = 0; i < this->size; i++)
                    new_sb[i] = this->sb[i];
                for(size_t i = 0; i < this->size; i++)
                    new_sb[i].init(this->size, this->alloc_size, this->v);
                size_t new_i = this->size + 1;
                this->size += this->alloc_size;
                delete [] this->sb;
                this->sb = new_sb;
                new_sb[new_i].x = index;
                new_sb[new_i].set_value(jndex, v);
                new_sb[end].next = new_i;
                return true;
            }

            void fill_normal(VALUE_T *array, size_t m)
            {
                size_t px = 0;
                do
                {
                    size_t py = 0;
                    do
                    {
                        array[this->sb[px].x * m + this->sb[px].sl[py].y] = this->sb[px].sl[py].v;
                        py = this->sb[px].sl[py].next;
                    } while(py != 0);
                    px = this->sb[px].next;
                } while(px != 0);
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
            sparseblock<VALUE_T> *_sparse;
            VALUE_T *_normal;
            VALUE_T v; // default value
            size_t c; // default value count
        public:
            matrix(size_t n, size_t m, VALUE_T v);
            ~matrix();
            VALUE_T get(size_t index, size_t jndex);
            void set(size_t index, size_t jndex, VALUE_T v);
    };

    template<typename VALUE_T>
    matrix<VALUE_T>::matrix(size_t n, size_t m, VALUE_T v)
    {
        this->n = n;
        this->m = m;
        this->v = v;
        this->c = n * m;
        this->status = STATUS_SPARSE;
        size_t n_size = n >> 4;
        size_t m_size = m >> 4;
        n_size = n_size ? n_size : 1;
        m_size = m_size ? m_size : 1;
        n_size = n_size < 0x400 ? n_size : 0x400;
        m_size = m_size < 0x400 ? m_size : 0x400;
        this->_sparse = new sparseblock<VALUE_T>(n_size, m_size, v);
        this->_normal = 0;
    }

    template<typename VALUE_T>
    VALUE_T matrix<VALUE_T>::get(size_t index, size_t jndex)
    {
        if(index > this->n || jndex > this->m)
            return this->v;
        if(status == STATUS_SPARSE)
        {
            return this->_sparse->get_value(index, jndex);
        }
    }

    template<typename VALUE_T>
    void matrix<VALUE_T>::set(size_t index, size_t jndex, VALUE_T v)
    {
        if(index > this->n || jndex > this->m)
            return;
        if(this->v == v)
            this->c++;
        if(this->status == STATUS_SPARSE)
        {
            if(this->_sparse->set_value(index, jndex, v))
            {
                if(this->v == v)
                    this->c++;
                else
                    this->c--;
            }
            if(this->c > (n * m) >> 2)
            {
                this->_normal = new VALUE_T[n * m];
                for(size_t i = 0; i < n; i++)
                    for(size_t j = 0; j < m; j++)
                        this->_normal[i * m + j] = this->v;
                this->_sparse->fill_normal(this->_normal, this->m);
                delete this->_sparse;
                this->_sparse = 0;
                this->status = STATUS_NORMAL;
            }
        }
        if(this->status == STATUS_SPARSE)
        {
            size_t offset = index * this->m + jndex;
            this->_normal[offset] = v;
            if(this->_normal[offset] == this->v)
            {
                if(v != this->v)
                    this->c--;
            }
            else
            {
                if(v == this->v)
                    this->c++;
            }
            if(this->c < (n * m) >> 4)
            {
                size_t n_size = n >> 2;
                size_t m_size = m >> 2;
                n_size = n_size ? n_size : 1;
                m_size = m_size ? m_size : 1;
                this->_sparse = new sparseblock<VALUE_T>(n_size, m_size, this->v);
                for(size_t i = 0; i < this->n; i++)
                    for(size_t j = 0; j < this->m; j++)
                    {
                        size_t offset = i * this->m + j;
                        if(this->_normal[offset] != this->v)
                            this->_sparse->set_value(i, j, this->_normal[offset]);
                    }
                delete [] this->_normal;
                this->_normal = 0;
                this->status = STATUS_SPARSE;
            }
        }
    }
}
