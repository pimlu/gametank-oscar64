#!/usr/bin/env python3
import os
import codegen


script_dir = os.path.dirname(os.path.abspath(__file__))
src_dir = os.path.join(script_dir, 'src')




def main():
    imul_const_file = os.path.join(src_dir, 'system', 'imul_const.cpp')
    with open(imul_const_file, 'w') as f:
        f.write(codegen.gen_imul_const())

    
    sin_lut_data_file = os.path.join(src_dir, 'geometry', 'sin_lut_data.cpp')
    with open(sin_lut_data_file, 'w') as f:
        f.write(codegen.gen_sin_lut_data())
    

if __name__ == '__main__':
    main()
