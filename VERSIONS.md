### Producer API

| Release      | API changed\*\* |  Protocol | Supported by server from/to | Status              |Comment|
| ------------ | ----------- | -------- | ------------------------- | --------------------- | ------- |
| 21.09.0      | No         |  v0.4     | 21.09.0/21.09.0           | current version              |beamline token for raw |
| 21.06.0      | Yes         |  v0.3     | 21.06.0/21.09.0           | deprecates 01.09.2022         |arbitrary characters|
| 21.03.3      | No          |  v0.2     | 21.03.2/21.09.0           | deprecates 01.07.2022        |bugfix in server|
| 21.03.2      | Yes         |  v0.2     | 21.03.2/21.09.0           | deprecates 01.07.2022        |bugfixes, add delete_stream|
| 21.03.1      | No          |  v0.1     | 21.03.0/21.09.0           | deprecates 01.06.2022   |bugfix in server|
| 21.03.0      | Yes         |  v0.1     | 21.03.0/21.09.0           |                 |          |

### Consumer API

| Release      | API changed\*\* |  Protocol | Supported by server from/to | Status         |Comment|
| ------------ | ----------- | --------- | ------------------------- | ---------------- | ------- |
| 21.09.0      | No         |  v0.4      | 21.06.0/21.09.0           | current version  | |
| 21.06.0      | Yes         |  v0.4     | 21.06.0/21.09.0           |   |arbitrary characters, bugfixes |
| 21.03.3      | Yes         |  v0.3     | 21.03.3/21.09.0           | deprecates 01.06.2022  |bugfix in server, error type for dublicated ack|
| 21.03.2      | Yes         |  v0.2     | 21.03.2/21.09.0           | deprecates 01.06.2022  |bugfixes, add delete_stream|
| 21.03.1      | No          |  v0.1     | 21.03.0/21.09.0           | deprecates 01.06.2022       |bugfix in server|
| 21.03.0      | Yes         |  v0.1     | 21.03.0/21.09.0           |                  |        |

\* insignificant changes/bugfixes (e.g. in return type, etc), normally do not require client code changes, but formally might break the client

\*\* under API change we understand any changes that may require updating/recompiling user code -
(e.g. parameter rename ,...),
adding new functionality (thus new client will not work with old server),
but also changing internal structures and client behavior (e.g. adding a field to a structure (privat or public), changing error type, ...).
Check CHANGELOG.md to see more details about changes.   