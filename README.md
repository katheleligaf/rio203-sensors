# rio203-sensors

- <https://its-just-nans.github.io/rio203/about>

## WebSocket Protocol


```mermaid
sequenceDiagram
    Note over Server,Device: Device connect to server
    Server ->> Device: {request: name}
    Device -->> Server: {response: name, name: ADRESSE_MAC, id: idPlace}
    Note right of Device: ⚠️ send ID only only if exists
```

```mermaid
sequenceDiagram
    Note over Server,Device: Device get it's ID
    Device -->> Server: {request: getId}
    Note left of Server: ⚠️ response only if ID exists
    Server ->>Device : {response: setId, id: idPlace} 
```


When we create a parking, the Device as an ID in the BDD and a MAC in the Device
We need to create a binding between them

```mermaid
sequenceDiagram
    Note over Server,Device: Binding an ID
    Server ->> Device: {request: setId, id: idPlace}
```


```mermaid
sequenceDiagram
    Note over Server,Device: When car hover device
    Device -->> Server: {request: info, name: ADRESSE_MAC, id: idPlace}
```


```mermaid
sequenceDiagram
    Note over Server,Device: When reserving a place
    Server ->> Device: {request: setState, state: reserved}
```


```mermaid
sequenceDiagram
    Note over Server,Device: Requesting State
    Server ->> Device: {request: state}
    Device -->> Server: {response: state, id: idPlace, name: ADRESSE MAC, state: CURRENT_STATE}
    Note right of Device: free
    Note right of Device: busy
    Note right of Device: reserved
    Note right of Device: error
```
