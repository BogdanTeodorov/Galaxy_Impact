// sfml.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
///
//  Student name:   Bogdan Teodorov
//  Student email:  bteodorov01@mynbcc.ca
//  Homework 
//     I certify that this work is my work only, any work copied from Stack Overflow, textbooks, 
//     or elsewhere is properly cited. 

#include "GameEngine.h"

int main()
{
    GameEngine game("../config.txt");
    game.run();
    return 0;
}