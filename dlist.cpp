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

#include        "dlist.h"

namespace Utils
{

/******************* DList, methods for ***************************/

void DList::insert(DListRec* rec)
{
    /* Put at the start of the list.
     * No need to remove from list first.
     */
    extract(rec);
    if (list_) rec->insert(list_);
    list_ = rec;
}

void DList::append(DListRec* rec)
{
    /* Put at the end of the list.
     * Do not need to remove from original list before append. 
     */
    extract(rec);
    if (list_) 
        rec->insert(list_);
    else 
        list_ = rec;
}

DListRec* DList::extractFirst()
{
    DListRec* rec = list_;
    if (rec) 
    {
        list_ = rec->next_;
        if (list_ == rec) list_ = 0;
        rec->remove();
    }
    return rec;
}

void DList::extract(DListRec *rec)
{
    /* if this is the first element in the list
     * we need to promote the next record to pole
     * position. This needs to be done before the
     * remove because remove resets a records next
     * and previous pointers.
     */
    
    if (rec) {
        if (list_ == rec) {
            list_ = rec->next_;
            if (list_ == rec) list_ = 0;
        }
        rec->remove();
    }
}

bool DList::member(DListRec* rec) const
{
    DListIterator i(*this);
    i.find(rec);
    return *i != 0;
}

unsigned int DList::size() const 
{
    /* Return the number of elements in this list */
    unsigned int         n = 0;
    DListRec*  l = list_;
    if (l) {
        do {
            ++n;
            l = l->next();
        } while (l != list_);
    }
    return n;
}


/***************** DListIterator, methods for ***********************/

DListIterator::DListIterator(const DList& list)
{
    init();
    list_ = (DList*)&list;
    reset();
}

DListIterator& DListIterator::operator=(const DListIterator& i)
{
    list_ = i.list_;
    pos_ = i.pos_;
    return *this;
}

DListIterator& DListIterator::operator++()
{
    /* Advance the iterator position to the next element of this list.
     * If advanced from the last element, the position will become 
     * invalid. If iterated when the position is invalid, the iterator
     * will move to the front of the list.
     */
    if (pos_) {
        DListRec* rec = pos_->next();
        if (rec == list_->first() || pos_ == rec) pos_ = 0;
        else pos_ = rec;
    }
    else pos_ = list_->first();
    return *this;
}

DListIterator& DListIterator::operator--()
{
    /* Step back one place of iteration. If we step back from the
     * first element, the position becomes invalid. If we step back
     * from the invalid position, the iterator is set to the end
     * of the list.
     */
    if (pos_) {
        pos_ = pos_->previous();
        if (pos_ == list_->last()) pos_ = 0;
    }
    else pos_ = list_->last();
    return *this;
}

void DListIterator::remove()
{
    /* Remove the element at the current iterator position.
     * The iterator will now point to the previous element of the list
     * or be invalid, if we removed the first member.
     */

    DListRec* rec = pos_;
    if (rec) 
    {
        --*this;
        list_->remove(rec);
    }
}

void DListIterator::extract()
{
    /* Remove the element at the current iterator position.
     * The iterator will now point to the previous element of the list
     * or be invalid, if we removed the first member.
     */

    DListRec* rec = pos_;
    if (rec) 
    {
        --*this;
        list_->extract(rec);
    }
}

void DListIterator::insert(DListRec* rec)
{
    /* Insert the given `rec'ord at this point, we will then
     * be pointing to the inserted record.
     */

    if (pos_) 
        list_->insertAt(rec, pos_);
    else 
        list_->insert(rec);
    pos_ = rec;
}

DListIterator& DListIterator::find(const DListRec* rec) 
{
    if (pos_) 
    {
        do {
            if (pos_ == rec) break;
        } while (*++*this);
    }
    return *this;
}


}; // Utils
