# Run it in 2025
- On a Linux machine
- Python 3.8 and conan 1.61. I installed the python 3.8 in a conda virtual environment. To make sure that your compiling action can find this conan, you can set up an environmental variable that includes the folder. E.g. `export PATH=/miniforge/envs/<env_name>/bin:$PATH` for my miniconda environment `<env_name>`.
- cmake.
- In the project folder, run the following command
```bash
cmake . -B out -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTING=OFF -DSGA_BUILD_HEADLESS=ON
cd out
/mnt/task_runtime/cmake-4.0.2-linux-x86_64/bin/cmake --build . -j 12
cd bin
./arena -configPath "../resources/gameConfigurations/TBS/ktk_elastic_rule.yaml" -logPath "test.yaml" -mapsPath "../resources/gameConfigurations/TBS/KtKMaps_kwah_test.yaml" -seed "2222"
```

# Stratega
[![Documentation Status](https://readthedocs.org/projects/stratega/badge/?version=latest)](https://stratega.readthedocs.io/en/latest/?badge=latest)
[![pypi releases](https://img.shields.io/pypi/v/stratega.svg)](https://pypi.org/project/stratega)
[![Builds](https://github.com/GAIGResearch/Stratega/actions/workflows/ci.yml/badge.svg?branch=dev)](https://github.com/GAIGResearch/Stratega/actions/workflows/ci.yml)
[![Python Wheel Builds](https://github.com/GAIGResearch/Stratega/actions/workflows/wheels.yml/badge.svg?branch=dev)](https://github.com/GAIGResearch/Stratega/actions/workflows/wheels.yml)

Stratega aims to provide a fast and flexible framework for researching AI in complex strategy games. Games are configured using YAML-files and can be played through a GUI or by agents using an API. Stratega allows creating a wide variety of turn-based and real-time strategy games. Due to the flexibility of using YAML-files, the user can design and run various games for testing agents without adjusting it to the game.

The framework has been built with a focus of statistical forward planning (SFP) agents. For this purpose, agents can access and modify game states and use the forward model to simulate the game. Thanks to the ability to configure a wide range of games and access to the forward model, Stratega is perfectly suited for researching general game playing in complex games.

# Community

Join the Discord community for help and to talk about what you are doing with Stratega!

[![Discord Chat](https://img.shields.io/discord/783231009738719233.svg)](https://discord.gg/Y2uZZ3TSuT)

## Documentation

Full documentation can be found here:

[stratega.readthedocs.io](https://stratega.readthedocs.io/)

# Gallery

TBS
![TBS](/images/tbsScreenshot.png)
RTS
![RTS](/images/rtsScreenshot.png)
