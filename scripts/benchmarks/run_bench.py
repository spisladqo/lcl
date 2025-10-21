import subprocess

NUM_RUNS = 40

PROG = "lcl"
CONV_LIST = ["rowwise"
            # ,"columnwise",
            # "pilewise",
            # . "pixelwise"
            ]
CONC_MODES = [
    # "seq",
            #   "par",
              "parq"
            ]

# change this for yourself
NTHREADS_LIST = [1, 2, 4, 8, 12]
JOBS_LIST = [
             "1,1,1,1", "1,1,2,1", "1,1,4,1", "1,1,8,1",
             "1,1,12,1",  "2,2,1,2",
             "2,2,2,2", "2,2,4,2", "2,2,6,2",
            "3,3,1,3", "3,3,2,3", "3,3,4,3", 
             ]

OUTPUT_FILE = "bench_res.txt"


mona_lisa_images = [
    "Mona_Lisa.bmp",
    "Mona_Lisa1.bmp", "Mona_Lisa2.bmp", "Mona_Lisa3.bmp",
        "Mona_Lisa4.bmp", "Mona_Lisa5.bmp", "Mona_Lisa6.bmp", "Mona_Lisa7.bmp",
            "Mona_Lisa8.bmp", "Mona_Lisa9.bmp", "Mona_Lisa10.bmp", "Mona_Lisa11.bmp",
                    ]

rye_images = ["Rye.bmp", "Rye1.bmp", "Rye2.bmp", "Rye3.bmp"]

ninth_wave_images = [
    "The_Ninth_Wave.bmp",
    # "The_Ninth_Wave1.bmp", "The_Ninth_Wave2.bmp", "The_Ninth_Wave3.bmp"
    # "The_Ninth_Wave4.bmp", "The_Ninth_Wave5.bmp", "The_Ninth_Wave6.bmp", "The_Ninth_Wave7.bmp"
    # "The_Ninth_Wave8.bmp", "The_Ninth_Wave9.bmp", "The_Ninth_Wave10.bmp", "The_Ninth_Wave11.bmp"
                     ]

parasol_images = ["Monet_Parasol.bmp"
                  , "Monet_Parasol1.bmp", "Monet_Parasol2.bmp", "Monet_Parasol3.bmp"
                  , "Monet_Parasol4.bmp", "Monet_Parasol5.bmp", "Monet_Parasol6.bmp"
                  , "Monet_Parasol7.bmp", "Monet_Parasol8.bmp", "Monet_Parasol9.bmp"
                  , "Monet_Parasol10.bmp", "Monet_Parasol11.bmp"
                  ]

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
            write_to_file("\nSequential")
            for i in range(NUM_RUNS):
                write_to_file(f"run {i}")
                for img_in, img_out in zip(images_in, images_out):
                    run_bench(mode, img_in, img_out, "blur", "", "", OUTPUT_FILE)

        elif mode == "par":
            write_to_file("\nParallel")
            for conv in CONV_LIST:
                write_to_file(f"Mode: {conv}")
                for nthreads in NTHREADS_LIST:
                    write_to_file(f"nthreads: {nthreads}")
                    for i in range(NUM_RUNS):
                        write_to_file(f"run {i}")
                        for img_in, img_out in zip(images_in, images_out):
                            run_bench(mode, img_in, img_out, "blur", conv, nthreads, OUTPUT_FILE)

        elif mode == "parq":
            write_to_file("\nParallel + Queue")
            for conv in CONV_LIST:
                write_to_file(f"Mode: {conv}")
                input_img_str = ",".join(images_in)
                output_img_str = ",".join(images_out)

                filterl = [filt for img in images_in]
                filters = ",".join(filterl)

                convsl = [conv for img in images_in]
                convs = ",".join(convsl)

                for nthreads in JOBS_LIST:
                    write_to_file(f"nthreads: {nthreads}")
                    for i in range(NUM_RUNS):
                        write_to_file(f"run {i}")
                        run_bench(mode, input_img_str, output_img_str, filters, convs, nthreads, OUTPUT_FILE)



run_scenarios()
