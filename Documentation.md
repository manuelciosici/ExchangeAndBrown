## File formats

<!----------------------------------------------->
### Table of Contents 

  * [Flat clustering](#markdown-header-flat-clustering)
<!--  * [Building](#markdown-header-building) -->
<!--  * [Usage](#markdown-header-usage)-->
<!--  * [License](#markdown-header-license)-->
<!--  * [Contact](#markdown-header-contact)-->
<!--  * [References](#markdown-header-references)-->


------------------------------------

# Flat clustering
The flat clustering format is a text file with one cluster per line. Each cluster is represented by its member word types separated by comma and space. The file is sorted from top to bottom from the more frequent to the least frequent cluster. And within the cluster, left to right from the most frequent to the least frequent word.

# Exchange JSON
The exchange runner can output its progress to a JSON file containing information about the AMI of each iteration, duration of each iteration (in milliseconds), how many swaps have been performed, and a few other details about the parameters used for the run.

Below is a sample of the JSON file:

> {
> 
>     "algorithm": "EXCHANGE_STEPS",
>     "ami_progression": [
>         0.34185137336487514,
>         1.1807175343429535,
>         1.2719326785423313,
>         1.2823230567026833,
>         1.2881291860627222,
>         1.291352535230331,
>         1.3008575181719202,
>         1.304579594617369,
>         1.3103957545758398,
>         1.3147285328865372,
>         1.3182900172506287
>     ],
>     "durations": [
>         5023,
>         4184,
>         4000,
>         3983,
>         3958,
>         3999,
>         3960,
>         3974,
>         3957,
>         3952
>     ],
>     "iterations_exchange": 10,
>     "num_clusters": 50,
>     "num_iterations": 10,
>     "swaps": [
>         25626,
>         7891,
>         2381,
>         1839,
>         990,
>         2314,
>         1163,
>         1657,
>         1185,
>         1003
>     ],
>     "total_words": 33904
> }
> 