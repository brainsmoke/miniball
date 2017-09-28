#!/bin/sh

rename s'/(.*?)-Edge.Cuts\..*/$1.GKO/' gerb/*
rename s'/(.*?)-.*\.(...)$/$1.\U$2/' gerb/*
rename s'/(.*?).drl/$1.TXT/' gerb/*

