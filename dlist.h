/**
 *
 * Copyright (c) 2010-2015 Voidware Ltd.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code as
 * defined in and that are subject to the Voidware Public Source Licence version
 * 1.0 (the 'Licence'). You may not use this file except in compliance with the
 * Licence or with expressly written permission from Voidware.  Please obtain a
 * copy of the Licence at http://www.voidware.com/legal/vpsl1.txt and read it
 * before using this file.
 * 
 * The Original Code and all software distributed under the Licence are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS
 * OR IMPLIED, AND VOIDWARE HEREBY DISCLAIMS ALL SUCH WARRANTIES, INCLUDING
 * WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 *
 * Please see the Licence for the specific language governing rights and 
 * limitations under the Licence.
 *
 * contact@voidware.com
 */

#ifndef __dlist_h__
#define __dlist_h__

namespace Utils
{

/* Forward Decls */
struct DList;

struct DListRec
{
    /* Link element class.
     * This mixes in the necessary pointers for the dlist when
     * inherited.
     */

    friend struct DList;
    // Constructors
			DListRec() { init(); }

    // Accessors
    DListRec*           next() const { return next_; }
    DListRec*           previous() const { return previous_; }
    bool                orphan() const { return next_ == this; }

  protected:

    void                init() { reset(); }
    void                reset() { next_ = this; previous_ = this; }
    void                remove()
                        {
                            previous_->next_ = next_;
                            next_->previous_ = previous_;
                            /* Tidy up our next and previous pointers
                             * to ourself. This protects against remove
                             * when not already in a list.
                             */
                            reset();
                        }
    void                insert(DListRec* rec)
                        {
                            next_ = rec;
                            previous_ = rec->previous_;
                            previous_->next_ = this;
                            next_->previous_ = this;
                        }
    void                removeAndInsert(DListRec* rec) 
                        {
                            previous_->next_ = next_;
                            next_->previous_ = previous_;
                            insert(rec);
                        }
    
  private:

    DListRec*           next_;
    DListRec*           previous_;
};

struct DList
{
    // Constructors
                        DList() { init(); }

    // Destructor
                        ~DList() { empty(); }

    // Accessors
    DListRec*           first() const { return list_; }
    DListRec*           last() const
                                { return list_ ? list_->previous() : 0; }

    // Features
    void                append(DListRec* rec);
    void                insert(DListRec* rec);
    void                insertAt(DListRec* rec, DListRec* atRec)
    {
        rec->removeAndInsert(atRec);
        if (list_ == atRec) list_ = rec;
    }
    void                add(DListRec* rec) { append(rec); }
    unsigned int        size() const;

    void                remove(DListRec* rec) { extract(rec); }
    void                extract(DListRec *rec);
    void                empty() { list_ = 0; }
    bool                member(DListRec*) const;
    bool                isEmpty() const { return list_ == 0; }
    DListRec*           extractFirst();

  protected:

    void                init() { list_ = 0; }

  private:

    DListRec*           list_;
};

struct DListIterator: public DListRec
{
    // Constructors
                                DListIterator() { init(); }
                                DListIterator(const DList& list);
                                DListIterator(const DListIterator& i)
                                {
                                    init();
                                    *this = i;
                                }

    // Destructor
                                ~DListIterator() {}

    // Features
    DListIterator&              operator=(const DListIterator&);

    DListIterator&              operator++();
    DListIterator&              operator--();
    DListRec*                   operator*() const { return pos_; }

    void                        remove();
    void                        extract();
    void                        insert(DListRec*);
    DListIterator&              find(const DListRec*);
    void                        invalidate() { pos_ = 0; }
    void                        reset() { pos_ = 0; operator++(); }
    
    int                         operator==(DListIterator& i) const
                                   { return pos_ == i.pos_; }

protected:

    void                        init()
    {
        pos_ = 0;
        list_ = 0;
    }

    DList*                      list_;
    DListRec*                   pos_;
};

};

#endif /* __dlist_h__ */

