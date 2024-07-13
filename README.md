# Prefender: A Prefetching Defender against Cache Side Channel Attacks as A Pretender
**Authors:** Luyi Li*, Jiayi Huang**, Lang Feng*, Zhongfeng Wang*

**Organization:** *Nanjing University, **UC Santa Barbara

**Original Version on DATE 2022:** [PDF](https://www.owenlly.top/uploads/date2022_prefender_paper.pdf)

**Extended Version on TC:** [PDF](https://www.owenlly.top/uploads/tc2024_prefender_paper.pdf)


# Compiled Proof-of-Concept
Link: https://drive.google.com/file/d/1h8EogDYHDDjZeObBVHhJy1xyP6MkNe19/view?usp=sharing

This compiled PoC is based on the extended version of the Prefender paper on TC, including the Record Protector(RP) to increase the robustness of original version.

In the shared Google drive directory, we include compiled gem5 executable files with both Prefender on and off.

# Example Usage

1. Download the compiled gem5 executable files from Google drive and decompress:
    ```
    pip3 install gdown
    gdown https://drive.google.com/uc?id=1h8EogDYHDDjZeObBVHhJy1xyP6MkNe19
    tar -xvf gem5_prefender.tar.xz
    ```

2. Go to `./attacks` and compile all the attack PoCs:
    ```
    cd attacks
    make
    ```
3. For example, to run the Flush+Reload attack on Prefender:

    **Default hardware prefetcher:** Stride Prefetcher
    ```
    ./gem5_prefender_on.opt configs/example/se.py --cpu-type=DerivO3CPU --caches --l2cache --l1d-hwp-type=StridePrefetcher -c ./attacks/flush_reload.out
    ```
    
    **Default hardware prefetcher:** Tagged Prefetcher
    ```
    ./gem5_prefender_on.opt configs/example/se.py --cpu-type=DerivO3CPU --caches --l2cache --l1d-hwp-type=TaggedPrefetcher -c ./attacks/flush_reload.out
    ```

# Expected Output
For example, a successful Flush+Reload attack can detect **only one cache hit** at the 65th element of the probe array (secret is 'A'):
```
index#60: 167
index#61: 167
index#62: 167
index#63: 167
index#64: 167
index#65: 49
index#66: 167
index#67: 167
index#68: 167
index#69: 196
```

However, Prefender can make the attack detect several cache hits, preventing it from extracting the secret:
```
index#60: 25
index#61: 25
index#62: 25
index#63: 25
index#64: 49
index#65: 25
index#66: 25
index#67: 25
index#68: 25
index#69: 204
```

# Citation
```
@inproceedings{li2022prefender,
  title={Prefender: A Prefetching Defender against Cache Side Channel Attacks as A Pretender},
  author={Luyi Li, Jiayi Huang, Lang Feng and Zhongfeng Wang},
  booktitle={2022 Design, Automation and Test in Europe (DATE 2022)},
  year={2022}
}
```

```
@article{li2024prefender_ext,
  title={Prefender: A Prefetching Defender against Cache Side Channel Attacks as A Pretender (Extended)},
  author={Luyi Li, Jiayi Huang, Lang Feng and Zhongfeng Wang},
  journal={IEEE Transactions on Computers},
  year={2024}
}
```