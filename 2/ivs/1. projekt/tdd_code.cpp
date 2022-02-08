//======== Copyright (c) 2021, FIT VUT Brno, All rights reserved. ============//
//
// Purpose:     Test Driven Development - priority queue code
//
// $NoKeywords: $ivs_project_1 $tdd_code.cpp
// $Author:     Miroslav Harag <xharag02@stud.fit.vutbr.cz>
// $Date:       $2021-01-04
//============================================================================//
/**
 * @file tdd_code.cpp
 * @author Miroslav Harag
 * 
 * @brief Implementace metod tridy prioritni fronty.
 */

#include <stdlib.h>
#include <stdio.h>

#include "tdd_code.h"

//============================================================================//
// ** ZDE DOPLNTE IMPLEMENTACI **
//
// Zde doplnte implementaci verejneho rozhrani prioritni fronty (Priority Queue)
// 1. Verejne rozhrani fronty specifikovane v: tdd_code.h (sekce "public:")
//    - Konstruktor (PriorityQueue()), Destruktor (~PriorityQueue())
//    - Metody Insert/Remove/Find a GetHead
//    - Pripadne vase metody definovane v tdd_code.h (sekce "protected:")
//
// Cilem je dosahnout plne funkcni implementace prioritni fronty implementovane
// pomoci tzv. "double-linked list", ktera bude splnovat dodane testy 
// (tdd_tests.cpp).
//============================================================================//

PriorityQueue::PriorityQueue()
{
    m_pHead = NULL;
}

PriorityQueue::~PriorityQueue()
{
    while(m_pHead != NULL)
    {
        //From head to tail
        Element_t *to_delete = m_pHead;
        m_pHead = m_pHead->pNext;
        free(to_delete);
    }
}

void PriorityQueue::Insert(int value)
{
    //Create new element
    Element_t *new_element = (Element_t *)malloc(sizeof(Element_t));
    new_element->value = value;

    //Find correct position for new_element
    Element_t **pos = &m_pHead;
    while(*pos != NULL && (*pos)->value > new_element->value)
    {
        pos = &(*pos)->pNext;
    }

    //Insert new_element 
    new_element->pNext = *pos;
    *pos = new_element;
}

bool PriorityQueue::Remove(int value)
{
    //Find element with correct value
    Element_t **pos = &m_pHead;
    while(*pos != NULL && (*pos)->value != value)
    {
        pos = &(*pos)->pNext;
    }

    //Remove first element with correct value
    if(*pos != NULL)
    {
        Element_t *to_delete = *pos;
        *pos = (*pos)->pNext;
        free(to_delete);
        return true;
    }

    return false;
}

PriorityQueue::Element_t *PriorityQueue::Find(int value)
{
    Element_t *pos = m_pHead;
    while(pos != NULL){
        if(pos->value == value){
            return pos;
        }
        pos = pos->pNext;
    }
    return NULL;
}

size_t PriorityQueue::Length()
{
    Element_t *pos = m_pHead;
    int length = 0;
    while(pos != NULL){
        length++;
        pos = pos->pNext;
    }
	return length;
}

PriorityQueue::Element_t *PriorityQueue::GetHead()
{
    return m_pHead;
}

/*** Konec souboru tdd_code.cpp ***/
