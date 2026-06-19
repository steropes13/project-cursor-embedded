# Check if the model header file is present, and if not, create it with the numb content
Import("env")
import os
MODEL_HEADER_FILE = "../scratch/model.h"

if not os.path.exists(MODEL_HEADER_FILE):
    env.Append(CPPDEFINES=["__MODEL_NOT_PRESENT__"])
    