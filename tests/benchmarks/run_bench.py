import subprocess

NUM_RUNS = 1

PROG = "lcl"
CONV_LIST = ["rowwise", "columnwise", "pilewise", "pixelwise"]
CONC_MODES = ["parq", "seq", "par"]

# change this for yourself
MAX_CORES = 12

NTHREADS_LIST = [2 * i for i in range(1, int(MAX_CORES/2))]
NTHREADS_LIST.insert(0, 1)

#change this as well
JOBS_LIST = ["1,1,1,1", "1,1,2,1", "1,1,4,1", "1,1,6,1", "1,1,8,1",
            "2,2,1,2", "2,2,2,2", "2,2,4,2", "2,2,6,2",
            "3,3,1,3", "3,3,2,3", "3,3,3,3",
            ]

OUTPUT_FILE = "bench_res.txt"

small_images = ["Mona_Lisa.bmp", "Almond_van_Gogh.bmp",
                "Impression_Sunrise.bmp", "Sunflowers_van_Gogh.bmp"]
medium_images = ["The_Ninth_Wave.bmp"]
big_images = ["Monet_parasol.bmp"]

INPUT_DIR = "images/input/"
OUTPUT_DIR = "images/output/"

input_f = lambda x : INPUT_DIR + x
output_f = lambda x : OUTPUT_DIR + x

small_images_in = [input_f(x) for x in small_images]
small_images_out = [output_f(x) for x in small_images]

big_images_in = [input_f(x) for x in big_images]
big_images_out = [output_f(x) for x in big_images]


def run_bench(mode: str, srcs: str, targs: str, filters: str,
            convs: str, nthreads: str, output_file: str):
    for i in range(NUM_RUNS):
        if mode == "seq":
            command = f"./{PROG} --mode={mode} --src={srcs} --targ={targs} --filter={filters} >> {output_file}"
        else:
            command = f"./{PROG} --mode={mode} --src={srcs} --targ={targs} --filter={filters} \
            --conv={convs} --nthreads={nthreads} >> {output_file}"
        subprocess.call(f"echo {command} >> {output_file}", shell=True)
        subprocess.call(command, shell=True)


def write_to_file(s):
    with open(OUTPUT_FILE, 'a') as f:
        f.write(s + "\n")


def main():
    images_in = small_images_in
    images_out = small_images_out

    open(OUTPUT_FILE, 'w')

    for mode in CONC_MODES:
        write_to_file(f"concurrency mode: {mode}")
        
        if mode == "seq":
            for img_in, img_out in zip(images_in, images_out):
                write_to_file(f"input: {img_in}, output: {img_out}")
                run_bench(mode, img_in, img_out, "blur", "", "", OUTPUT_FILE)

        elif mode == "par":
            for conv in CONV_LIST:
                write_to_file(f"conv mode: {conv}")
                for img_in, img_out in zip(images_in, images_out):
                    write_to_file(f"input: {img_in}, output: {img_out}")
                    for nthreads in NTHREADS_LIST:
                        write_to_file(f"nthreads: {nthreads}")
                        run_bench(mode, img_in, img_out, "blur", conv, nthreads, OUTPUT_FILE)

        elif mode == "parq":
            for conv in CONV_LIST:
                write_to_file(f"conv mode: {conv}")
                input_img_str = ",".join(images_in)
                output_img_str = ",".join(images_out)
                write_to_file(f"input: {input_img_str}, output: {output_img_str}")
                for nthreads in JOBS_LIST:
                    write_to_file(f"nthreads: {nthreads}")
                    run_bench(mode, input_img_str, output_img_str, "blur", conv, nthreads, OUTPUT_FILE)

main()