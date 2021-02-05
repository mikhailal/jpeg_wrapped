#!/usr/bin/python3

import jpeg_manipulator
import argparse

def main():
    parser = argparse.ArgumentParser(description='Set input/output file and % of result\'s intensity (Y component, float, 0.0-1.0)')
    parser.add_argument('--inputfile', required=True)
    parser.add_argument('--outputfile', required=True)
    parser.add_argument('--AX', required=True)
    parser.add_argument('--AY', required=True)
    parser.add_argument('--BX', required=True)
    parser.add_argument('--BY', required=True)
    parser.add_argument('--intensity', help='Float value between 0.0 and 1.0', required=True)
    args = parser.parse_args()

    if float(args.intensity) > 1.0 or float(args.intensity) < 0.0:
        print('error: invalid intensity value, legal range 0.0-1.0')
        raise ValueError('invalid intensity value')

    current_manip = jpeg_manipulator.JpegManipulator()
    current_manip.LoadFromJpegFile(args.inputfile)
    info = current_manip.GetMetadata()
    if not(info.buffer_is_valid):
        raise OSError("LibJPEG wrapper: buffer status is illegal")
    buf = current_manip.GetDataBuffer()
    newdata = []
    # Y U V Y, distance = info.components
    fl_intensity = float(args.intensity)
    A_id = (int(args.AY)*info.width+int(args.AX))*info.components
    B_id = (int(args.BY)*info.width+int(args.BX))*info.components

    row_count = info.components*info.width

    for i in range(A_id, B_id, info.components):
        if (i%row_count)//info.components in range(int(args.AX), int(args.BX)):
            current_manip[i] = int(current_manip[i] * fl_intensity)

    current_manip.SaveToJpegFile(args.outputfile)

if __name__ == "__main__":
   main()
