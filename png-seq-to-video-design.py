import os
import argparse
from pathlib import Path
import glob
import cv2

def getStem(path):
    return Path(path).stem

def sorted_paths_from_folder(folder,start=0,n=None,recursive=False):
    paths = []
    
    paths += glob.glob(f'{folder}/*.png')
    paths += glob.glob(f'{folder}/*.tiff')

    if recursive:
        paths += glob.glob(f'{folder}/**/*.png')
        paths += glob.glob(f'{folder}/**/*.tiff')

    paths.sort(key=getStem)

    paths = paths[start:]

    n = n or len(paths)
    
    paths = paths[:n]

    return paths

def make_video_variations(input,output,n_frames):

    output = Path(output) / Path(input).name
    output.mkdir(exist_ok=True)

    # COPY OVER PNG SEQ
    files = sorted_paths_from_folder(input,0,n_frames,False)

    full_size_frames_path = Path(output / 'frames')
    full_size_frames_path.mkdir(exist_ok=True)

    for file in files:
        os.system(f'cp "{file}" "{full_size_frames_path}"')

    # MAKE SMALL IMAGES
    small_frames_path = Path(output / 'mini-frames')
    small_frames_path.mkdir(exist_ok=True)

    img = cv2.imread(files[0])
    width = img.shape[1]
    height = img.shape[0]
    x = int((width - height) / 2)

    n_digits = len(Path(files[0]).stem)

    # crop to center square
    os.system(f'ffmpeg -i "{full_size_frames_path}/%0{n_digits}d.png" -vf "crop={height}:{height}:{x}:0" -start_number 0 "{small_frames_path}/%0{n_digits}d.png"')
    # scale down to 32x32
    os.system(f'ffmpeg -i "{small_frames_path}/%0{n_digits}d.png" -vf scale=32:32 -start_number 0 "{small_frames_path}/%0{n_digits}d.png"')

    # CONVERT TO HAP (ALPHA)
    os.system(f'ffmpeg -framerate 30 -f image2 -i "{full_size_frames_path}/%0{n_digits}d.png" -vcodec hap -format hap_alpha -pix_fmt rgba "{output}/hap.mov"')

    # MAKE MP4 OF SMALL IMAGES
    os.system(f'ffmpeg -framerate 30 -pattern_type glob -i "{small_frames_path}/*.png" -c:v libx264 -pix_fmt yuv420p "{output}/mini.mp4"')

    # MAKE MP4 OF ORIGINAL FOR REFERENCE
    os.system(f'ffmpeg -framerate 30 -f image2 -i "{full_size_frames_path}/%0{n_digits}d.png" -c:v libx264 -pix_fmt yuv420p "{output}/h264.mp4"')

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--png-seq',required=True,nargs='+',type=str,help='a folder or list of folders containing png sequences for transforming')
    parser.add_argument('-o','--output',required=True,type=str)
    parser.add_argument('--n-frames',required=False,default=-1,type=int)
    args = parser.parse_args()

    for input in args.png_seq:
        make_video_variations(input,args.output,args.n_frames)