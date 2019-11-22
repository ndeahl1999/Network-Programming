========TEST1========
Invocation:
    python3 -u hw4.py 0 9000 2
    python3 -u hw4.py 1 9001 2

Inputs:
    -->peer00: BOOTSTRAP peer01 9001
    -->peer00: STORE 5 apples
    -->peer00: FIND_VALUE 5
    -->peer00: QUIT
    -->peer01: QUIT

peer00 output:
    After BOOTSTRAP(1), k_buckets now look like:
    0: 1:9001
    1:
    2:
    3:
    Storing key 5 at node 1
    Before FIND_VALUE command, k-buckets are:
    0: 1:9001
    1:
    2:
    3:
    Found value "apples" for key 5
    After FIND_VALUE command, k-buckets are:
    0: 1:9001
    1:
    2:
    3:
    Letting 1 know I'm quitting.
    Shut down node 0

peer01 output:
    Serving FindNode(0) request for 0
    Storing key 5 value "apples"
    Serving FindKey(5) request for 0
    Evicting quitting node 0 from bucket 0
    Shut down node 1

========TEST2========
Invocation:
    python3 -u hw4.py 0 9000 2
    python3 -u hw4.py 8 9001 2

Inputs:
    -->peer08: BOOTSTRAP peer00 9000
    -->peer00: STORE 1 apples
    -->peer00: FIND_VALUE 1
    -->peer08: FIND_VALUE 5
    -->peer00: FIND_VALUE 5
    -->peer08: FIND_VALUE 1
    -->peer00: QUIT
    -->peer08: QUIT

peer00 output:
    Serving FindNode(8) request for 8
    Storing key 1 at node 0
    Before FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 8:9001
    Found data "apples" for key 1
    Serving FindKey(5) request for 8
    Before FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 8:9001
    Could not find key 5
    After FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 8:9001
    Serving FindKey(1) request for 8
    Letting 8 know I'm quitting.
    Shut down node 0

peer08 output:
    After BOOTSTRAP(0), k_buckets now look like:
    0:
    1:
    2:
    3: 0:9000
    Before FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 0:9000
    Could not find key 5
    After FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 0:9000
    Serving FindKey(5) request for 0
    Before FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 0:9000
    Found value "apples" for key 1
    After FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 0:9000
    Evicting quitting node 0 from bucket 3
    Shut down node 8

========TEST3========
Invocation:
    python3 -u hw4.py 0 9000 2
    python3 -u hw4.py 8 9001 2
    python3 -u hw4.py 11 9002 2

Inputs:
    -->peer00: BOOTSTRAP peer08 9001
    -->peer00: STORE 9 apples
    -->peer00: FIND_VALUE 9
    -->peer08: FIND_VALUE 9
    -->peer11: BOOTSTRAP peer00 9000
    -->peer00: STORE 11 carrots
    -->peer08: FIND_VALUE 11
    -->peer11: FIND_VALUE 9
    -->peer11: STORE 9 bananas
    -->peer11: FIND_VALUE 9
    -->peer08: FIND_VALUE 9
    -->peer00: FIND_VALUE 9
    -->peer00: QUIT
    -->peer08: QUIT
    -->peer11: QUIT

peer00 output:
    After BOOTSTRAP(8), k_buckets now look like:
    0:
    1:
    2:
    3: 8:9001
    Storing key 9 at node 8
    Before FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 8:9001
    Found value "apples" for key 9
    After FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 8:9001
    Serving FindNode(11) request for 11
    Storing key 11 at node 11
    Serving FindKey(11) request for 8
    Before FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 11:9002 8:9001
    Found value "bananas" for key 9
    After FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 11:9002 8:9001
    Letting 11 know I'm quitting.
    Letting 8 know I'm quitting.
    Shut down node 0

peer08 output:
    Serving FindNode(0) request for 0
    Storing key 9 value "apples"
    Serving FindKey(9) request for 0
    Before FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 0:9000
    Found data "apples" for key 9
    Before FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 0:9000
    Found value "carrots" for key 11
    After FIND_VALUE command, k-buckets are:
    0:
    1: 11:9002
    2:
    3: 0:9000
    Serving FindKey(9) request for 11
    Storing key 9 value "bananas"
    Serving FindKey(9) request for 11
    Before FIND_VALUE command, k-buckets are:
    0:
    1: 11:9002
    2:
    3: 0:9000
    Found data "bananas" for key 9
    Serving FindKey(9) request for 0
    Evicting quitting node 0 from bucket 3
    Letting 11 know I'm quitting.
    Shut down node 8

peer11 output:
    After BOOTSTRAP(0), k_buckets now look like:
    0:
    1: 8:9001
    2:
    3: 0:9000
    Storing key 11 value "carrots"
    Serving FindKey(11) request for 8
    Before FIND_VALUE command, k-buckets are:
    0:
    1: 8:9001
    2:
    3: 0:9000
    Found value "apples" for key 9
    After FIND_VALUE command, k-buckets are:
    0:
    1: 8:9001
    2:
    3: 0:9000
    Storing key 9 at node 8
    Before FIND_VALUE command, k-buckets are:
    0:
    1: 8:9001
    2:
    3: 0:9000
    Found value "bananas" for key 9
    After FIND_VALUE command, k-buckets are:
    0:
    1: 8:9001
    2:
    3: 0:9000
    Evicting quitting node 0 from bucket 3
    Evicting quitting node 8 from bucket 1
    Shut down node 11

========TEST4========
Invocation:
    python3 -u hw4.py 0 9000 2
    python3 -u hw4.py 8 9001 2
    python3 -u hw4.py 9 9002 2
    python3 -u hw4.py 10 9003 2

Inputs:
    -->peer00: BOOTSTRAP peer08 9001
    -->peer09: BOOTSTRAP peer08 9001
    -->peer10: BOOTSTRAP peer08 9001
    -->peer00: STORE 9 apples
    -->peer00: FIND_NODE 9
    -->peer00: STORE 9 bananas
    -->peer08: FIND_VALUE 9
    -->peer00: FIND_VALUE 9
    -->peer09: FIND_VALUE 9
    -->peer00: STORE 8 carrots
    -->peer00: STORE 10 dates
    -->peer00: FIND_VALUE 10
    -->peer00: FIND_VALUE 9
    -->peer00: FIND_VALUE 8
    -->peer00: FIND_VALUE 10
    -->peer00: QUIT
    -->peer08: QUIT
    -->peer09: QUIT
    -->peer10: QUIT

peer00 output:
    After BOOTSTRAP(8), k_buckets now look like:
    0:
    1:
    2:
    3: 8:9001
    Storing key 9 at node 8
    Before FIND_NODE command, k-buckets are:
    0:
    1:
    2:
    3: 8:9001
    Found destination id 9
    After FIND_NODE command, k-buckets are:
    0:
    1:
    2:
    3: 9:9002 10:9003
    Storing key 9 at node 9
    Before FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 9:9002 10:9003
    Found value "bananas" for key 9
    After FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 10:9003 9:9002
    Storing key 8 at node 9
    Storing key 10 at node 10
    Before FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 10:9003 9:9002
    Found value "dates" for key 10
    After FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 9:9002 10:9003
    Before FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 9:9002 10:9003
    Found value "bananas" for key 9
    After FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 10:9003 9:9002
    Before FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 10:9003 9:9002
    Found value "carrots" for key 8
    After FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 10:9003 9:9002
    Before FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 10:9003 9:9002
    Found value "dates" for key 10
    After FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 9:9002 10:9003
    Letting 9 know I'm quitting.
    Letting 10 know I'm quitting.
    Shut down node 0

peer08 output:
    Serving FindNode(0) request for 0
    Serving FindNode(9) request for 9
    Serving FindNode(10) request for 10
    Storing key 9 value "apples"
    Serving FindNode(9) request for 0
    Before FIND_VALUE command, k-buckets are:
    0: 9:9002
    1: 10:9003
    2:
    3: 0:9000
    Found data "apples" for key 9
    Letting 9 know I'm quitting.
    Letting 10 know I'm quitting.
    Letting 0 know I'm quitting.
    Shut down node 8

peer09 output:
    After BOOTSTRAP(8), k_buckets now look like:
    0: 8:9001
    1:
    2:
    3: 0:9000
    Storing key 9 value "bananas"
    Serving FindKey(9) request for 0
    Before FIND_VALUE command, k-buckets are:
    0: 8:9001
    1:
    2:
    3: 0:9000
    Found data "bananas" for key 9
    Storing key 8 value "carrots"
    Serving FindKey(9) request for 0
    Serving FindKey(8) request for 0
    Evicting quitting node 0 from bucket 3
    Evicting quitting node 8 from bucket 0
    Shut down node 9

peer10 output:
    After BOOTSTRAP(8), k_buckets now look like:
    0:
    1: 8:9001 9:9002
    2:
    3: 0:9000
    Storing key 10 value "dates"
    Serving FindKey(10) request for 0
    Serving FindKey(10) request for 0
    Evicting quitting node 0 from bucket 3
    Evicting quitting node 8 from bucket 1
    Letting 9 know I'm quitting.
    Shut down node 10

========TEST5========
Invocation:
    python3 -u hw4.py 0 9000 2
    python3 -u hw4.py 1 9001 2
    python3 -u hw4.py 3 9002 2
    python3 -u hw4.py 8 9003 2
    python3 -u hw4.py 9 9004 2

Inputs:
    -->peer01: BOOTSTRAP peer00 9000
    -->peer03: BOOTSTRAP peer00 9000
    -->peer08: BOOTSTRAP peer00 9000
    -->peer09: BOOTSTRAP peer00 9000
    -->peer00: STORE 0 apples
    -->peer00: FIND_VALUE 0
    -->peer01: FIND_VALUE 0
    -->peer03: STORE 0 bananas
    -->peer03: FIND_VALUE 0
    -->peer08: FIND_VALUE 0
    -->peer09: FIND_NODE 3
    -->peer09: STORE 3 carrots
    -->peer09: STORE 9 dates
    -->peer00: FIND_VALUE 3
    -->peer01: FIND_VALUE 3
    -->peer08: FIND_NODE 9
    -->peer08: FIND_VALUE 9
    -->peer00: FIND_VALUE 0
    -->peer08: STORE 9 eggplant
    -->peer01: FIND_VALUE 9
    -->peer03: FIND_NODE 8
    -->peer03: FIND_VALUE 9
    -->peer03: STORE 8 fennel
    -->peer03: FIND_VALUE 8
    -->peer03: FIND_VALUE 8
    -->peer00: QUIT
    -->peer01: QUIT
    -->peer03: QUIT
    -->peer08: QUIT
    -->peer09: QUIT

peer00 output:
    Serving FindNode(1) request for 1
    Serving FindNode(3) request for 3
    Serving FindNode(8) request for 8
    Serving FindNode(9) request for 9
    Storing key 0 at node 0
    Before FIND_VALUE command, k-buckets are:
    0: 1:9001
    1: 3:9002
    2:
    3: 8:9003 9:9004
    Found data "apples" for key 0
    Serving FindKey(0) request for 1
    Storing key 0 value "bananas"
    Serving FindKey(0) request for 3
    Serving FindNode(3) request for 9
    Before FIND_VALUE command, k-buckets are:
    0: 1:9001
    1: 3:9002
    2:
    3: 8:9003 9:9004
    Found value "carrots" for key 3
    After FIND_VALUE command, k-buckets are:
    0: 1:9001
    1: 3:9002
    2:
    3: 8:9003 9:9004
    Serving FindKey(3) request for 1
    Before FIND_VALUE command, k-buckets are:
    0: 1:9001
    1: 3:9002
    2:
    3: 8:9003 9:9004
    Found data "bananas" for key 0
    Letting 1 know I'm quitting.
    Letting 3 know I'm quitting.
    Letting 8 know I'm quitting.
    Letting 9 know I'm quitting.
    Shut down node 0

peer01 output:
    After BOOTSTRAP(0), k_buckets now look like:
    0: 0:9000
    1:
    2:
    3:
    Before FIND_VALUE command, k-buckets are:
    0: 0:9000
    1:
    2:
    3:
    Found value "apples" for key 0
    After FIND_VALUE command, k-buckets are:
    0: 0:9000
    1:
    2:
    3:
    Serving FindKey(0) request for 8
    Serving FindNode(3) request for 9
    Before FIND_VALUE command, k-buckets are:
    0: 0:9000
    1:
    2:
    3: 8:9003 9:9004
    Found value "carrots" for key 3
    After FIND_VALUE command, k-buckets are:
    0: 0:9000
    1: 3:9002
    2:
    3: 8:9003 9:9004
    Serving FindNode(9) request for 8
    Before FIND_VALUE command, k-buckets are:
    0: 0:9000
    1: 3:9002
    2:
    3: 9:9004 8:9003
    Found value "eggplant" for key 9
    After FIND_VALUE command, k-buckets are:
    0: 0:9000
    1: 3:9002
    2:
    3: 8:9003 9:9004
    Evicting quitting node 0 from bucket 0
    Letting 3 know I'm quitting.
    Letting 8 know I'm quitting.
    Letting 9 know I'm quitting.
    Shut down node 1

peer03 output:
    After BOOTSTRAP(0), k_buckets now look like:
    0:
    1: 0:9000 1:9001
    2:
    3:
    Storing key 0 at node 0
    Before FIND_VALUE command, k-buckets are:
    0:
    1: 0:9000 1:9001
    2:
    3:
    Found value "bananas" for key 0
    After FIND_VALUE command, k-buckets are:
    0:
    1: 1:9001 0:9000
    2:
    3:
    Serving FindKey(0) request for 8
    Storing key 3 value "carrots"
    Serving FindKey(3) request for 0
    Serving FindKey(3) request for 1
    Serving FindNode(9) request for 8
    Before FIND_NODE command, k-buckets are:
    0:
    1: 0:9000 1:9001
    2:
    3: 9:9004 8:9003
    Found destination id 8
    After FIND_NODE command, k-buckets are:
    0:
    1: 0:9000 1:9001
    2:
    3: 8:9003 9:9004
    Before FIND_VALUE command, k-buckets are:
    0:
    1: 0:9000 1:9001
    2:
    3: 8:9003 9:9004
    Found value "eggplant" for key 9
    After FIND_VALUE command, k-buckets are:
    0:
    1: 0:9000 1:9001
    2:
    3: 8:9003 9:9004
    Storing key 8 at node 8
    Before FIND_VALUE command, k-buckets are:
    0:
    1: 0:9000 1:9001
    2:
    3: 8:9003 9:9004
    Found value "fennel" for key 8
    After FIND_VALUE command, k-buckets are:
    0:
    1: 0:9000 1:9001
    2:
    3: 9:9004 8:9003
    Before FIND_VALUE command, k-buckets are:
    0:
    1: 0:9000 1:9001
    2:
    3: 9:9004 8:9003
    Found value "fennel" for key 8
    After FIND_VALUE command, k-buckets are:
    0:
    1: 0:9000 1:9001
    2:
    3: 9:9004 8:9003
    Evicting quitting node 0 from bucket 1
    Evicting quitting node 1 from bucket 1
    Letting 9 know I'm quitting.
    Letting 8 know I'm quitting.
    Shut down node 3

peer08 output:
    After BOOTSTRAP(0), k_buckets now look like:
    0:
    1:
    2:
    3: 1:9001 3:9002
    Before FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 1:9001 3:9002
    Could not find key 0
    After FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 3:9002 1:9001
    Before FIND_NODE command, k-buckets are:
    0:
    1:
    2:
    3: 3:9002 1:9001
    Found destination id 9
    After FIND_NODE command, k-buckets are:
    0: 9:9004
    1:
    2:
    3: 1:9001 3:9002
    Before FIND_VALUE command, k-buckets are:
    0: 9:9004
    1:
    2:
    3: 1:9001 3:9002
    Found value "dates" for key 9
    After FIND_VALUE command, k-buckets are:
    0: 9:9004
    1:
    2:
    3: 1:9001 3:9002
    Storing key 9 at node 9
    Serving FindNode(8) request for 3
    Storing key 8 value "fennel"
    Serving FindKey(8) request for 3
    Serving FindKey(8) request for 3
    No record of quitting node 0 in k-buckets.
    Evicting quitting node 1 from bucket 3
    Evicting quitting node 3 from bucket 3
    Letting 9 know I'm quitting.
    Shut down node 8

peer09 output:
    After BOOTSTRAP(0), k_buckets now look like:
    0: 8:9003
    1:
    2:
    3: 0:9000 1:9001
    Before FIND_NODE command, k-buckets are:
    0: 8:9003
    1:
    2:
    3: 0:9000 1:9001
    Found destination id 3
    After FIND_NODE command, k-buckets are:
    0: 8:9003
    1:
    2:
    3: 3:9002 1:9001
    Storing key 3 at node 3
    Storing key 9 at node 9
    Serving FindKey(3) request for 1
    Serving FindKey(9) request for 8
    Storing key 9 value "eggplant"
    Serving FindKey(9) request for 1
    Serving FindNode(8) request for 3
    Serving FindKey(9) request for 3
    No record of quitting node 0 in k-buckets.
    Evicting quitting node 1 from bucket 3
    Evicting quitting node 3 from bucket 3
    Evicting quitting node 8 from bucket 0
    Shut down node 9

========TEST6========
Invocation:
    python3 -u hw4.py 0 9000 2
    python3 -u hw4.py 1 9001 2
    python3 -u hw4.py 8 9002 2

Inputs:
    -->peer00: BOOTSTRAP peer01 9001
    -->peer01: BOOTSTRAP peer00 9000
    -->peer00: STORE 8 apples
    -->peer01: FIND_VALUE 8
    -->peer00: FIND_VALUE 8
    -->peer01: STORE 9 bananas
    -->peer01: FIND_VALUE 9
    -->peer00: FIND_VALUE 9
    -->peer08: BOOTSTRAP peer00 9000
    -->peer08: FIND_VALUE 9
    -->peer08: STORE 9 carrots
    -->peer00: FIND_VALUE 9
    -->peer01: FIND_VALUE 9
    -->peer08: FIND_VALUE 9
    -->peer00: FIND_VALUE 8
    -->peer01: FIND_VALUE 8
    -->peer08: FIND_VALUE 8
    -->peer00: QUIT
    -->peer01: QUIT
    -->peer08: QUIT

peer00 output:
    After BOOTSTRAP(1), k_buckets now look like:
    0: 1:9001
    1:
    2:
    3:
    Serving FindNode(1) request for 1
    Storing key 8 at node 0
    Serving FindKey(8) request for 1
    Before FIND_VALUE command, k-buckets are:
    0: 1:9001
    1:
    2:
    3:
    Found data "apples" for key 8
    Before FIND_VALUE command, k-buckets are:
    0: 1:9001
    1:
    2:
    3:
    Found value "bananas" for key 9
    After FIND_VALUE command, k-buckets are:
    0: 1:9001
    1:
    2:
    3:
    Serving FindNode(8) request for 8
    Before FIND_VALUE command, k-buckets are:
    0: 1:9001
    1:
    2:
    3: 8:9002
    Found value "carrots" for key 9
    After FIND_VALUE command, k-buckets are:
    0: 1:9001
    1:
    2:
    3: 8:9002
    Before FIND_VALUE command, k-buckets are:
    0: 1:9001
    1:
    2:
    3: 8:9002
    Found data "apples" for key 8
    Serving FindKey(8) request for 1
    Letting 1 know I'm quitting.
    Letting 8 know I'm quitting.
    Shut down node 0

peer01 output:
    Serving FindNode(0) request for 0
    After BOOTSTRAP(0), k_buckets now look like:
    0: 0:9000
    1:
    2:
    3:
    Before FIND_VALUE command, k-buckets are:
    0: 0:9000
    1:
    2:
    3:
    Found value "apples" for key 8
    After FIND_VALUE command, k-buckets are:
    0: 0:9000
    1:
    2:
    3:
    Storing key 9 at node 1
    Before FIND_VALUE command, k-buckets are:
    0: 0:9000
    1:
    2:
    3:
    Found data "bananas" for key 9
    Serving FindKey(9) request for 0
    Serving FindKey(9) request for 8
    Before FIND_VALUE command, k-buckets are:
    0: 0:9000
    1:
    2:
    3: 8:9002
    Found data "bananas" for key 9
    Before FIND_VALUE command, k-buckets are:
    0: 0:9000
    1:
    2:
    3: 8:9002
    Found value "apples" for key 8
    After FIND_VALUE command, k-buckets are:
    0: 0:9000
    1:
    2:
    3: 8:9002
    Evicting quitting node 0 from bucket 0
    Letting 8 know I'm quitting.
    Shut down node 1

peer08 output:
    After BOOTSTRAP(0), k_buckets now look like:
    0:
    1:
    2:
    3: 0:9000 1:9001
    Before FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 0:9000 1:9001
    Found value "bananas" for key 9
    After FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 0:9000 1:9001
    Storing key 9 at node 8
    Serving FindKey(9) request for 0
    Before FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 1:9001 0:9000
    Found data "carrots" for key 9
    Serving FindKey(8) request for 1
    Before FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 0:9000 1:9001
    Could not find key 8
    After FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3: 0:9000 1:9001
    Evicting quitting node 0 from bucket 3
    Evicting quitting node 1 from bucket 3
    Shut down node 8

========TEST7========
Invocation:
    python3 -u hw4.py 0 9000 2
    python3 -u hw4.py 1 9001 2

Inputs:
    -->peer00: BOOTSTRAP peer01 9001
    -->peer00: STORE 1 apples
    -->peer00: FIND_VALUE 1
    -->peer01: FIND_VALUE 1
    -->peer01: QUIT
    -->peer00: FIND_VALUE 1
    -->peer00: QUIT
    -->peer01: QUIT

peer00 output:
    After BOOTSTRAP(1), k_buckets now look like:
    0: 1:9001
    1:
    2:
    3:
    Storing key 1 at node 1
    Before FIND_VALUE command, k-buckets are:
    0: 1:9001
    1:
    2:
    3:
    Found value "apples" for key 1
    After FIND_VALUE command, k-buckets are:
    0: 1:9001
    1:
    2:
    3:
    Evicting quitting node 1 from bucket 0
    Before FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3:
    Could not find key 1
    After FIND_VALUE command, k-buckets are:
    0:
    1:
    2:
    3:
    Shut down node 0

peer01 output:
    Serving FindNode(0) request for 0
    Storing key 1 value "apples"
    Serving FindKey(1) request for 0
    Before FIND_VALUE command, k-buckets are:
    0: 0:9000
    1:
    2:
    3:
    Found data "apples" for key 1
    Letting 0 know I'm quitting.
    Shut down node 1
