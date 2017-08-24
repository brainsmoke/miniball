#!/bin/bash

while ! make flash TARGET="$1"; do echo ; done
