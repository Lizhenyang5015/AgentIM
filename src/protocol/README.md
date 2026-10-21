# protocol

Data protocols exchanged between the client and server: Protobuf/JSON definitions, encoding, decoding, and protocol-version compatibility.

Protocol models are separate from application models: protocols may change, while application entities in `src/model` should remain as stable as possible.
