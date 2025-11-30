#!/usr/bin/env python3
import os
import codegen.imul_const as imul_const
import codegen.fixed as fixed


script_dir = os.path.dirname(os.path.abspath(__file__))
src_dir = os.path.join(script_dir, 'src')
imul_const_file = os.path.join(src_dir, 'system', 'imul_const.cpp')




def main():
    with open(imul_const_file, 'w') as f:
        f.write(imul_const.gen_imul_const())
    

if __name__ == '__main__':
    main()
