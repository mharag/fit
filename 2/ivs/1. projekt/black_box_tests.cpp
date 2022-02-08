//======== Copyright (c) 2017, FIT VUT Brno, All rights reserved. ============//
//
// Purpose:     Red-Black Tree - public interface tests
//
// $NoKeywords: $ivs_project_1 $black_box_tests.cpp
// $Author:     Miroslav Harag <xharag02@stud.fit.vutbr.cz>
// $Date:       $2021-03-10$
//============================================================================//
/**
 * @file black_box_tests.cpp
 * @author Miroslav Harag 
 * 
 * @brief Implementace testu binarniho stromu.
 */

#include <vector>

#include "gtest/gtest.h"

#include "red_black_tree.h"

//============================================================================//
// ** ZDE DOPLNTE TESTY **
//
// Zde doplnte testy Red-Black Tree, testujte nasledujici:
// 1. Verejne rozhrani stromu
//    - InsertNode/DeleteNode a FindNode
//    - Chovani techto metod testuje pro prazdny i neprazdny strom.
// 2. Axiomy (tedy vzdy platne vlastnosti) Red-Black Tree:
//    - Vsechny listove uzly stromu jsou *VZDY* cerne.
//    - Kazdy cerveny uzel muze mit *POUZE* cerne potomky.
//    - Vsechny cesty od kazdeho listoveho uzlu ke koreni stromu obsahuji
//      *STEJNY* pocet cernych uzlu.
//============================================================================//
class NonEmptyTree : public ::testing::Test
{
protected:
    virtual void SetUp() {
        int values[] = {0,10,-214,421,65,0,12,-32,13,15,98,123,518,12};

        for(int i = 0; i < 14; ++i){
            tree.InsertNode(values[i]);
        }
    }
    BinaryTree tree;
};

class EmptyTree : public ::testing::Test
{
protected:
    BinaryTree tree;
};

class TreeAxioms : public ::testing::Test
{
protected:
    virtual void SetUp() {
        int values[] = {10,-214,421,65,-32,13,15,98,123,518,12,0,1,90};
        for(int i = 0; i < 14; ++i){
            tree1.InsertNode(values[i]);
            tree2.InsertNode(values[i]);
        }
        tree2.DeleteNode(10);
        tree2.DeleteNode(15);
        tree2.DeleteNode(123);
        tree2.DeleteNode(98);
        tree2.DeleteNode(65);
        tree2.DeleteNode(-214);

    }
    BinaryTree tree1; 
    BinaryTree tree2; //Controls tree axioms after a couple of deletions
};

TEST_F(EmptyTree, InsertNode)
{
    //adding first element
    std::pair<bool, Node_t *> result = tree.InsertNode(1);
    EXPECT_TRUE(result.first);
    EXPECT_EQ(result.second->key, 1);
    EXPECT_EQ(tree.FindNode(1), result.second);
}

TEST_F(EmptyTree, DeleteNode)
{
    //deleting not existing element
    EXPECT_FALSE(tree.DeleteNode(1));
}

TEST_F(EmptyTree, FindNode)
{
    //find not existing element
    EXPECT_EQ(tree.FindNode(1), nullptr);
}

TEST_F(NonEmptyTree, InsertNode)
{
    //adding new element
    std::pair<bool, Node_t *> result = tree.InsertNode(1);
    EXPECT_TRUE(result.first);
    EXPECT_EQ(result.second->key, 1);
    EXPECT_EQ(tree.FindNode(1), result.second);

    //adding already existing element
    result = tree.InsertNode(10);
    EXPECT_FALSE(result.first);
    EXPECT_EQ(result.second->key, 10);
}

TEST_F(NonEmptyTree, DeleteNode)
{
    //deleting not existing element
    EXPECT_FALSE(tree.DeleteNode(1));

    //deleting existing element
    EXPECT_TRUE(tree.DeleteNode(10));
    EXPECT_EQ(tree.FindNode(10), nullptr);
}

TEST_F(NonEmptyTree, FindNode)
{
    //find not existing element
    EXPECT_EQ(tree.FindNode(1), nullptr);

    //find existing element
    EXPECT_EQ(tree.FindNode(10)->key, 10);
}

TEST_F(TreeAxioms, Axiom1)
{
    std::vector<Node_t *> outLeafNodes;
    tree1.GetLeafNodes(outLeafNodes);

    bool axiom_1 = true;
    for(int i = 0; i < outLeafNodes.size(); i++)
    {
        if(outLeafNodes[i]->color == RED)
        {
            axiom_1 = false;
            break;
        }
    }
    EXPECT_TRUE(axiom_1);

    tree2.GetLeafNodes(outLeafNodes);
    axiom_1 = true;
    for(int i = 0; i < outLeafNodes.size(); i++)
    {
        if(outLeafNodes[i]->color == RED)
        {
            axiom_1 = false;
            break;
        }
    }
    EXPECT_TRUE(axiom_1);
}

TEST_F(TreeAxioms, Axiom2)
{
    std::vector<Node_t *> outAllNodes;
    tree1.GetAllNodes(outAllNodes);

    bool axiom_2 = true;
    for(int i = 0; i < outAllNodes.size(); i++)
    {
        printf("%d\n", i);
        if(outAllNodes[i]->color == RED)
        {
            if((outAllNodes[i]->pLeft != NULL && outAllNodes[i]->pLeft->color == RED) 
            || (outAllNodes[i]->pRight != NULL && outAllNodes[i]->pRight->color == RED))
            {
                axiom_2 = false;
                break;
            }
        }
    }
    EXPECT_TRUE(axiom_2);

    tree2.GetAllNodes(outAllNodes);
    axiom_2 = true;
    for(int i = 0; i < outAllNodes.size(); i++)
    {
        if(outAllNodes[i]->color == RED)
        {
            if((outAllNodes[i]->pLeft != NULL && outAllNodes[i]->pLeft->color == RED) 
            || (outAllNodes[i]->pRight != NULL && outAllNodes[i]->pRight->color == RED))
            {
                axiom_2 = false;
                break;
            }
        }
    }
    EXPECT_TRUE(axiom_2);
}

TEST_F(TreeAxioms, Axiom3)
{
    std::vector<Node_t *> outLeafNodes;
    tree1.GetLeafNodes(outLeafNodes);

    bool axiom_3 = true;
    int number_of_black = -1;
    for(int i = 0; i < outLeafNodes.size(); i++)
    {
        Node_t *parent = outLeafNodes[i]->pParent;
        int counter = 0;
        while(parent != NULL)
        {
            if(parent->color == BLACK)
                counter++;
            parent = parent->pParent;
        }
        if(number_of_black != -1 && number_of_black != counter)
        {
            axiom_3 = false;
            break;
        }
        number_of_black = counter;
    }
    EXPECT_TRUE(axiom_3);

    tree2.GetLeafNodes(outLeafNodes);
    axiom_3 = true;
    number_of_black = -1;
    for(int i = 0; i < outLeafNodes.size(); i++)
    {
        Node_t *parent = outLeafNodes[i]->pParent;
        int counter = 0;
        while(parent != NULL)
        {
            if(parent->color == BLACK)
                counter++;
            parent = parent->pParent;
        }
        if(number_of_black != -1 && number_of_black != counter)
        {
            axiom_3 = false;
            break;
        }
        number_of_black = counter;
    }
    EXPECT_TRUE(axiom_3);
}

/*** Konec souboru black_box_tests.cpp ***/
