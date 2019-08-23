clc
clear all
close all
input = csvread('InputMatrix.csv');
Inverted = csvread('InvertedMatrix.csv');
res=input*Inverted;
M=round(res,1);
%this mulRes.csv be an identity matrix if matrix is invertable 
csvwrite('mulRes.csv',M);