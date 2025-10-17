import subprocess

NUM_RUNS = 50

PROG = "lcl"
CONV_LIST = ["rowwise", "columnwise", "pilewise", "pixelwise"]
CONC_MODES = ["seq", "par", "parq"]

# change this for yourself
MAX_CORES = 12

# NTHREADS_LIST = [2 * i for i in range(1, int(MAX_CORES/2))]
# NTHREADS_LIST.insert(0, 1)
NTHREADS_LIST = [12]

#change this as well (readers,foremen,workers,writers)
# JOBS_LIST = ["3,2,4,3"]
JOBS_LIST = ["2,2,6,2"]

OUTPUT_FILE = "bench_res.txt"

# small_images = ["Mona_Lisa.bmp", "Almond_van_Gogh.bmp",
#                 "Impression_Sunrise.bmp", "Sunflowers_van_Gogh.bmp"]
mona_lisa_images = ["Mona_Lisa.bmp", "Mona_Lisa1.bmp", "Mona_Lisa2.bmp", "Mona_Lisa3.bmp"]

rye_images = ["Rye.bmp", "Rye1.bmp", "Rye2.bmp", "Rye3.bmp"]

ninth_wave_images = ["The_Ninth_Wave.bmp", "The_Ninth_Wave1.bmp", "The_Ninth_Wave2.bmp", "The_Ninth_Wave3.bmp"]

parasol_images = ["Monet_Parasol.bmp", "Monet_Parasol1.bmp", "Monet_Parasol2.bmp", "Monet_Parasol3.bmp"]

INPUT_DIR = "images/input/"
OUTPUT_DIR = "images/output/"

input_f = lambda x : INPUT_DIR + x
output_f = lambda x : OUTPUT_DIR + x

mona_lisa_images_in = [input_f(x) for x in mona_lisa_images]
mona_lisa_images_out = [output_f(x) for x in mona_lisa_images]

rye_images_in = [input_f(x) for x in rye_images]
rye_images_out = [output_f(x) for x in rye_images]

ninth_wave_images_in = [input_f(x) for x in ninth_wave_images]
ninth_wave_images_out = [output_f(x) for x in ninth_wave_images]

big_images_in = [input_f(x) for x in parasol_images]
big_images_out = [output_f(x) for x in parasol_images]


def run_bench(mode: str, srcs: str, targs: str, filters: str,
            convs: str, nthreads: str, output_file: str):
    if mode == "seq":
        command = f"./{PROG} --mode={mode} --src={srcs} --targ={targs} --filter={filters} >> {output_file}"
    else:
        command = f"./{PROG} --mode={mode} --src={srcs} --targ={targs} --filter={filters} \
        --conv={convs} --nthreads={nthreads} >> {output_file}"
    # subprocess.call(f"echo {command} >> {output_file}", shell=True)

    # for i in range(NUM_RUNS):
    subprocess.call(command, shell=True)

    write_to_file("")


def write_to_file(s):
    with open(OUTPUT_FILE, 'a') as f:
        f.write(s + "\n")


def run_scenarios():
    images_in = mona_lisa_images_in
    images_out = mona_lisa_images_out

    filt = "blur"

    open(OUTPUT_FILE, 'w')

    for mode in CONC_MODES:
        
        if mode == "seq":
            write_to_file("Sequential")
            for i in range(NUM_RUNS):
                write_to_file(f"run {i}")
                for img_in, img_out in zip(images_in, images_out):
                    run_bench(mode, img_in, img_out, "blur", "", "", OUTPUT_FILE)

        elif mode == "par":
            write_to_file("Parallel")
            for conv in CONV_LIST:
                write_to_file(f"Mode: {conv}")
                for nthreads in NTHREADS_LIST:
                    for i in range(NUM_RUNS):
                        write_to_file(f"run {i}")
                        for img_in, img_out in zip(images_in, images_out):
                            run_bench(mode, img_in, img_out, "blur", conv, nthreads, OUTPUT_FILE)

        elif mode == "parq":
            write_to_file("Parallel + Queue")
            for conv in CONV_LIST:
                write_to_file(f"Mode: {conv}")
                if len(images_in) > 1:
                    input_img_str = ",".join(images_in)
                    output_img_str = ",".join(images_out)

                    filterl = [filt for img in images_in]
                    filters = ",".join(filterl)

                    convsl = [conv for img in images_in]
                    convs = ",".join(convsl)
                else:
                    input_img_str = images_in
                    output_img_str = images_out

                    filters = filt

                    convs = conv
                for nthreads in JOBS_LIST:
                    for i in range(NUM_RUNS):
                        write_to_file(f"run {i}")
                        run_bench(mode, input_img_str, output_img_str, filters, convs, nthreads, OUTPUT_FILE)



run_scenarios()
