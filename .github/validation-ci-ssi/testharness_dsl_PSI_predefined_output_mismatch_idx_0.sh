# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

<<com
PSI sample

Exemplifies one of the main usages of the PSI Workload. Tests the PSI workload for offline and latency by using
all the possible types along with even and odd sizes for 'k'. In this case it uses a config file that contains
a predefined (expected) output that will not match.
com

# server paths
CLEARTEXTLIB=$(realpath ./lib/libhebench_example_backend.so)
TESTHARNESS=$(realpath ./bin/test_harness)

# IDs relevant for offline and latency, covering all the types.
for id in 48 49 50 51 52 53 54 55; do
  # testing even and odd k
  for k in 33 34; do

dataset="input, 0, 2, local, $k , 0
afghanistan,,albania,,algeria,,andorra,,angola,,antigua and barbuda,,argentina,,armenia,,australia,,austria,,azerbaijan,,bahamas,,bahrain,,bangladesh,,barbados,,belarus,,belgium,,belize,,benin,,bhutan,,bolivia,,bosnia and herzegovina,,botswana,,brazil,,brunei,,bulgaria,,burkina faso,,burundi,,cabo verde,,cambodia,,cameroon,,canada,,central african republic,,chad,,chile,,china,,colombia,,comoros,,congo,,costa rica,,côte d’ivoire,,croatia,,cuba,,cyprus,,czech republic,,denmark,,djibouti,,dominica,,dominican republic,,east timor,,ecuador,,egypt,,el salvador,,equatorial guinea,,eritrea,,estonia,,eswatini,,ethiopia,,fiji,,finland,,france,,gabon,,gambia,,georgia,,germany,,ghana,,greece,,grenada,,guatemala,,guinea,,guinea-bissau,,guyana,,haiti,,honduras,,hungary,,iceland,,india,,indonesia,,iran,,iraq,,ireland,,israel,,italy,,jamaica,,japan,,jordan,,kazakhstan,,kenya,,kiribati,,north korea,,south korea,,kosovo,,kuwait,,kyrgyzstan,,laos,,latvia,,lebanon,,lesotho,,liberia,,libya,,liechtenstein,,lithuania,,luxembourg,,madagascar,,malawi,,malaysia,,maldives,,mali,,malta,,marshall islands,,mauritania,,mauritius,,mexico,,micronesia,,moldova,,monaco,,mongolia,,montenegro,,morocco,,mozambique,,myanmar,,burma,,namibia,,nauru,,nepal,,netherlands,,new zealand,,nicaragua,,niger,,nigeria,,north macedonia,,norway,,oman,,pakistan,,palau,,panama,,papua new guinea,,paraguay,,peru,,philippines,,poland,,portugal,,qatar,,romania,,russia,,rwanda,,saint kitts and nevis,,saint lucia,,saint vincent and the grenadines,,samoa,,san marino,,sao tome and principe,,saudi arabia,,senegal,,serbia,,seychelles,,sierra leone,,singapore,,slovakia,,slovenia,,solomon islands,,somalia,,south africa,,spain,,sri lanka,,sudan,,south sudan,,suriname,,sweden,,switzerland,,syria,,taiwan,,tajikistan,,tanzania,,thailand,,togo,,tonga,,trinidad and tobago,,tunisia,,turkey,,turkmenistan,,tuvalu,,uganda,,ukraine,,united arab emirates,,united kingdom,,united states,,uruguay,,uzbekistan,,vanuatu,,vatican city,,venezuela,,vietnam,,yemen,,zambia,,zimbabwe
albania,,andorra,,antigua and barbuda,,armenia,,austria,,bahamas,,bangladesh,,belarus,,belize,,bhutan,,bosnia and herzegovina,,brazil,,bulgaria,,burundi,,cambodia,,canada,,chad,,china,,comoros,,costa rica,,croatia,,cyprus,,denmark,,dominica,,east timor,,egypt,,equatorial guinea,,estonia,,ethiopia,,finland,,gabon,,georgia,,ghana,,grenada,,guinea,,guyana,,honduras,,iceland,,indonesia,,iraq,,israel,,jamaica,,jordan,,kenya,,north korea,,kosovo,,kyrgyzstan,,latvia,,lesotho,,libya,,lithuania,,madagascar,,malaysia,,mali,,marshall islands,,mauritius,,micronesia,,monaco,,montenegro,,mozambique,,burma,,nauru,,netherlands,,nicaragua,,nigeria,,norway,,pakistan,,panama,,paraguay,,philippines,,portugal,,romania,,rwanda,,saint lucia,,samoa,,sao tome and principe,,senegal,,seychelles,,singapore,,slovenia,,somalia,,spain,,sudan,,suriname,,switzerland,,taiwan,,tanzania,,togo,,trinidad and tobago,,turkey,,tuvalu,,ukraine,,united kingdom,,uruguay,,vanuatu,,venezuela,,yemen,,zimbabwe,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0

input, 1, 3 , local, $k , 0
iran,,marshall islands,,maldives,,fiji,,ecuador,,belgium,,norway,,burma,,congo,,jamaica,,portugal,,eritrea,,spain,,brunei,,croatia
sri lanka,,bahamas,,turkey,,namibia,,cambodia,,palau,,ghana,,denmark,,seychelles,,east timor,,0,,0,,0,,0,,0
nothing,,nothing,,nothing,,nothing,,nothing,,nothing,,nothing,,nothing,,nothing,,nothing,,nothing,,nothing,,nothing,,nothing,,nothing

output, 0, 6 , local, $k , 0
belgium_MADE_UP,,brunei,,congo,,croatia,,ecuador,,eritrea,,fiji,,iran,,jamaica,,maldives,,marshall islands,,burma,,norway,,portugal,,spain
bahamas,,cambodia,,denmark,,east timor,,ghana,,namibia,,palau,,seychelles,,sri lanka,,turkey,,0,,0,,0,,0,,0
0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0
croatia,,jamaica,,marshall islands,,burma,,norway,,portugal,,spain,,0,,0,,0,,0,,0,,0,,0,,0
bahamas,,cambodia,,denmark,,east timor,,ghana,,seychelles,,turkey,,0,,0,,0,,0,,0,,0,,0,,0
0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0,,0
"

    dataset_file=countries.csv
    echo "$dataset" > "$(pwd)/$dataset_file"

data="default_min_test_time: 0
default_sample_size: 0
random_seed: 1234
benchmark:
  - ID: $id
    dataset: $(pwd)/$dataset_file
    default_min_test_time: 0
    default_sample_sizes:
      0: 0
      1: 0
    params:
      0:
        name: n
        type: UInt64
        value:
          from: 196
          to: 196
          step: 0
      1:
        name: m
        type: UInt64
        value:
          from: 15
          to: 15
          step: 0
      2:
        name: k
        type: UInt64
        value:
          from: $k
          to: $k
          step: 0"

      file=psi.yaml
      echo "$data" > "$(pwd)/$file"

      "$TESTHARNESS" --backend_lib_path "$CLEARTEXTLIB" --config_file "$(pwd)/$file" | tee test_harness_output.log
      tmp="Failed indices, (ground truth; 0..$((k-1))), (output; $((14*k))..$((15*k-1)))"
      if ! grep -Fxq "$tmp" test_harness_output.log
      then
          echo "Failed to test the PSI's worload with: $0"
          echo "Workload ID: $id"
          echo "k: $k"
          # clean-up
          rm "$(pwd)/$file"
          rm "$(pwd)/$dataset_file"
          exit 1
      else
          echo "Successfully tested Test Harness' PSI with:"
          echo "Workload ID: $id"
          echo "k: $k"
      fi
    done # k related for   
done # ID related for

# clean-up
rm "$(pwd)/$file"
rm "$(pwd)/$dataset_file"

exit 0
