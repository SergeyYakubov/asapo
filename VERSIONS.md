### Producer API

| Release      | API changed | Breaking changes | Protocol | Supported by server from/to | Status              |Comment|
| ------------ | ----------- |----------------- | -------- | ------------------------- | --------------------- | ------- |
| 21.03.3      | No          | No               | v0.2     | 21.03.2/21.03.3           | current version        |bugfix in server|
| 21.03.2      | Yes         | No               | v0.2     | 21.03.2/21.03.3           | current version        |bugfixes, add delete_stream|
| 21.03.1      | No          | No               | v0.1     | 21.03.0/21.03.3           | deprecates 01.06.2022   |bugfix in server|
| 21.03.0      | Yes         | Yes              | v0.1     | 21.03.0/21.03.3           |                 |          |

### Consumer API

| Release      | API changed | Breaking changes | Protocol | Supported by server from/to | Status         |Comment|
| ------------ | ----------- |----------------- | -------- | ------------------------- | ---------------- | ------- |
| 21.03.3      | Yes         | No*              | v0.3     | 21.03.3/21.03.3           | current version  |bugfix in server, error type for dublicated ack|
| 21.03.2      | Yes         | No               | v0.2     | 21.03.2/21.03.3           | deprecates 01.06.2022  |bugfixes, add delete_stream|
| 21.03.1      | No          | No               | v0.1     | 21.03.0/21.03.3           | deprecates 01.06.2022       |bugfix in server|
| 21.03.0      | Yes         | Yes              | v0.1     | 21.03.0/21.03.3           |                  |        |

\* insignificant changes/bugfixes (e.g. in return type, etc), normally do not require client code changes, but formally might break the client