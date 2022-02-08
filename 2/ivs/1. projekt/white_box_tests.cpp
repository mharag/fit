//======== Copyright (c) 2021, FIT VUT Brno, All rights reserved. ============//
//
// Purpose:     White Box - Tests suite
//
// $NoKeywords: $ivs_project_1 $white_box_code.cpp
// $Author:     Miroslav Harag <xharag02@stud.fit.vutbr.cz>
// $Date:       $2021-03-10
//============================================================================//
/**
 * @file white_box_tests.cpp
 * @author Miroslav Harag
 * 
 * @brief Implementace testu prace s maticemi.
 */

#include "gtest/gtest.h"
#include "white_box_code.h"

//============================================================================//
// ** ZDE DOPLNTE TESTY **
//
// Zde doplnte testy operaci nad maticemi. Cilem testovani je:
// 1. Dosahnout maximalniho pokryti kodu (white_box_code.cpp) testy.
// 2. Overit spravne chovani operaci nad maticemi v zavislosti na rozmerech 
//    matic.
//============================================================================//

class MatrixTest: public ::testing::Test
{
    protected:
        virtual void SetUp() 
        {
            std::vector<std::vector<double>> values_3x2{{1,2},{3,4},{5,6}};
            std::vector<std::vector<double>> values_3x2_1{{2,4},{6,8},{10,12}};
            std::vector<std::vector<double>> values_1x9{{0,-1,1.8,9.74,420,-152.534,-215,342,0}};
            std::vector<std::vector<double>> values_9x1{{0},{-1},{1.8},{9.74},{420},{-152.534},{-215},{342},{0}};
            std::vector<std::vector<double>> values_9x9{{0,-1,1.8,9.7,420,-152.534,-215,342,0},
                {0,-1,1.8,4,420,19,-215,342,15},
                {-9,-64,1.8,9.74,40,-152.534,-215,51,0},
                {0,65,1.8,125.74,420,-2.67,15,342,0},
                {17,-1,1.8,9.74,16,5534,215,3642,15},
                {0,-1,1.8,9.74,942,-154,-25,153,0},
                {0,-0,1.8,24,51,-152.534,-43,3442,4685},
                {0,8,1.8,9.74,420,0,-215.223,342,0},
                {0,-1,1.8,9.74,69,-152.534,-215,342,-1563}};

            matrix_2x2 = Matrix(2,2);
            matrix_2x2.set({{1,2},{3,4}});

            matrix_3x3 = Matrix(3,3);
            matrix_3x3.set({{1,2,3},{3,4,5},{7,8,9}});

            matrix_3x2 = Matrix(3,2);
            matrix_3x2.set(values_3x2);

            matrix_3x2_1 = Matrix(3,2);
            matrix_3x2_1.set(values_3x2_1);

            matrix_1x9 = Matrix(1,9);
            matrix_1x9.set(values_1x9);

            matrix_9x1 = Matrix(9,1);
            matrix_9x1.set(values_9x1);

            matrix_9x9 = Matrix(9,9);
            matrix_9x9.set(values_9x9);

            matrix_9x9_copy = Matrix(9,9);
            matrix_9x9_copy.set(values_9x9);
        }
        Matrix matrix_1x1;
        Matrix matrix_2x2;
        Matrix matrix_3x3;
        Matrix matrix_3x2;
        Matrix matrix_3x2_1;
        Matrix matrix_9x9;
        Matrix matrix_9x9_copy;
        Matrix matrix_1x9;
        Matrix matrix_9x1;
};

TEST_F(MatrixTest, Constructor)
{
    //empty matrix 1x1
    Matrix new_matrix1;
    EXPECT_EQ(new_matrix1.get(0,0), 0);
    //empty matrix 4x5
    Matrix new_matrix2(4,5);
    EXPECT_EQ(new_matrix2.get(3,4), 0);
    //invalid matrix
    EXPECT_THROW(Matrix new_matrix3(0,0), std::runtime_error );
}

TEST_F(MatrixTest, Get)
{
    //valid get
    EXPECT_EQ(matrix_9x1.get(8,0), 0);
    //invalid get
    EXPECT_THROW(matrix_1x9.get(0,9), std::runtime_error );
}

TEST_F(MatrixTest, Set)
{
    //valid single set
    matrix_9x9.set(4,7, 10.125);
    EXPECT_EQ(matrix_9x9.get(4,7), 10.125);
    //valid full set
    EXPECT_TRUE(matrix_2x2.set({{1,7},{-11,2}}));
    EXPECT_EQ(matrix_2x2.get(1,0), -11);
    //invalid single set
    EXPECT_FALSE(matrix_1x9.set(1,1, 10.125));
    //invalid full set
    EXPECT_FALSE(matrix_1x1.set({{1,2},{1,2}}));
}

TEST_F(MatrixTest, Operator_equal)
{
    //different sizes
    EXPECT_THROW(matrix_1x9 == matrix_1x1, std::runtime_error );
    //same matrices
    EXPECT_TRUE(matrix_9x9 == matrix_9x9_copy);
    //different matrices
    matrix_9x9_copy.set(3,3,9);
    EXPECT_FALSE(matrix_9x9 == matrix_9x9_copy);
}

TEST_F(MatrixTest, Operator_plus)
{
    //different sizes
    EXPECT_THROW(matrix_9x1 + matrix_1x1, std::runtime_error );
    //matrices with same size
    Matrix result = matrix_3x2 + matrix_3x2_1;
    EXPECT_EQ(result.get(1,1),12);
    EXPECT_EQ(result.get(2,0),15);
    EXPECT_EQ(result.get(2,1),18);
}

TEST_F(MatrixTest, Operator_multiplication)
{
    //different sizes
    EXPECT_THROW(matrix_3x2 * matrix_9x9, std::runtime_error );
    //matrices with same size
    Matrix result = matrix_1x9 * matrix_9x1;
    EXPECT_DOUBLE_EQ(result.get(0,0),362954.728756);
    //matrix * number
    result = matrix_3x2 * 2;
    EXPECT_TRUE(result == matrix_3x2_1);
}

TEST_F(MatrixTest, Transpose)
{
    //9x9 matrix
    Matrix result = matrix_9x9.transpose();
    EXPECT_EQ(result.get(1,1),-1);
    EXPECT_EQ(result.get(8,1),15);
    //twice transposed matrix should be the same
    result = result.transpose();
    EXPECT_TRUE(result == matrix_9x9);
}


TEST_F(MatrixTest, Inverse)
{
    //singular matrix
    EXPECT_THROW(matrix_3x3.inverse(), std::runtime_error );
    //too big matrix
    EXPECT_THROW(matrix_9x9.inverse(), std::runtime_error );
    //matrix 2x2
    Matrix result = matrix_2x2.inverse() * matrix_2x2;
    EXPECT_NEAR(result.get(0,0),1,1e-15);
    EXPECT_NEAR(result.get(0,1),0,1e-15);
    EXPECT_NEAR(result.get(1,0),0,1e-15);
    EXPECT_NEAR(result.get(1,1),1,1e-15);
    //matrix 3x3
    matrix_3x3.set(1,2,10);
    result = matrix_3x3.inverse() * matrix_3x3;
    EXPECT_NEAR(result.get(1,1),1,1e-15);
    EXPECT_NEAR(result.get(2,2),1,1e-15);
    EXPECT_NEAR(result.get(1,2),0,1e-15);
    EXPECT_NEAR(result.get(0,2),0,1e-15);
}

TEST_F(MatrixTest, SolveEquation)
{
    //invalid sizes
    EXPECT_THROW(matrix_3x2.solveEquation({1,2}), std::runtime_error);
    EXPECT_THROW(matrix_3x3.solveEquation({0,0,0,0}), std::runtime_error );
    //9x9 with zero discriminant
    EXPECT_THROW(matrix_9x9.solveEquation({0,0,0,0,0,0,0,0,0}), std::runtime_error );
    //1x1 matrix
    matrix_1x1.set(0,0,1);
    EXPECT_EQ(matrix_1x1.solveEquation({1})[0],1);
    //2x2 with non-zero discriminant
    std::vector<double> result = matrix_2x2.solveEquation({0,2});
    EXPECT_EQ(result[0],2);
    EXPECT_EQ(result[1],-1);
    //3x3 with non-zero discriminant
    matrix_3x3.set(1,2,4);
    result = matrix_3x3.solveEquation({1,1,1});
    EXPECT_EQ(result[0],-1);
    EXPECT_EQ(result[1],1);
    EXPECT_EQ(result[2],0);

}

/*** Konec souboru white_box_tests.cpp ***/
