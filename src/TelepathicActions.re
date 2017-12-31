type linkId = string;

type userName = string;

type t =
  | MessageSend(linkId, userName, string)
  | MessageReceive(userName, string)
  | ClientRegister(linkId);

/**
    * Retrieve the textual key for the action
    */
let key = (action: t) =>
  switch action {
  | MessageSend(_linkId, _userName, _text) => "MESSAGE_SEND"
  | MessageReceive(_userName, _text) => "MESSAGE_RECEIVE"
  | ClientRegister(_linkId) => "CLIENT_REGISTER"
  };

/**
   * Transform a js action to an Action type
   */
let decode = (json: Js.Json.t) : option(t) => {
  open Js.Option;
  let opt = Js.Json.decodeObject(json);
  switch opt {
  | Some((dict: Js.Dict.t(Js.Json.t))) =>
    switch (Js.Dict.get(dict, "key")) {
    | Some(key) =>
      switch (Js.Json.decodeString(key)) {
      | Some("MESSAGE_SEND") =>
        switch (Js.Dict.get(dict, "payload")) {
        | Some(payload) =>
          let userName = Js.Dict.get(Js.Json.decodeObject(payload) |> getExn, "userName");
          switch userName {
          | Some(userName) =>
            switch (Js.Json.decodeString(userName)) {
            | Some(userName) =>
              let text = Js.Dict.get(Js.Json.decodeObject(payload) |> getExn, "text");
              switch text {
              | Some(text) =>
                switch (Js.Json.decodeString(text)) {
                | Some(text) =>
                  let linkId = Js.Dict.get(Js.Json.decodeObject(payload) |> getExn, "linkId");
                  switch linkId {
                  | Some(linkId) =>
                    switch (Js.Json.decodeString(linkId)) {
                    | Some(linkId) => Some(MessageSend(linkId, userName, text))
                    | None => None
                    }
                  | None => None
                  }
                | None => None
                }
              | None => None
              }
            | None => None
            }
          | None => None
          }
        | None => None
        }
      | Some("MESSAGE_RECEIVE") =>
        switch (Js.Dict.get(dict, "payload")) {
        | Some(payload) =>
          let userName = Js.Dict.get(Js.Json.decodeObject(payload) |> getExn, "userName");
          switch userName {
          | Some(userName) =>
            switch (Js.Json.decodeString(userName)) {
            | Some(userName) =>
              let text = Js.Dict.get(Js.Json.decodeObject(payload) |> getExn, "text");
              switch text {
              | Some(text) =>
                switch (Js.Json.decodeString(text)) {
                | Some(text) => Some(MessageReceive(userName, text))
                | None => None
                }
              | None => None
              }
            | None => None
            }
          | None => None
          }
        | None => None
        }
      | Some("CLIENT_REGISTER") =>
        switch (Js.Dict.get(dict, "payload")) {
        | Some(payload) =>
          let linkId = Js.Dict.get(Js.Json.decodeObject(payload) |> getExn, "linkId");
          switch linkId {
          | Some(linkId) =>
            switch (Js.Json.decodeString(linkId)) {
            | Some(linkId) => Some(ClientRegister(linkId))
            | None => None
            }
          | None => None
          }
        | None => None
        }
      | _ => None
      }
    | None => None
    }
  | None => Js.Exn.raiseError("Unable to parse action")
  }
};

let encode = (action: t) : Js.Json.t =>
  Json.Encode.(
    object_([
      ("key", action |> key |> string),
      (
        "payload",
        switch action {
        | MessageSend(linkId, userName, text) =>
          object_([
            ("linkId", string(linkId)),
            ("userName", string(userName)),
            ("text", string(text))
          ])
        | MessageReceive(userName, text) =>
          object_([("userName", string(userName)), ("text", string(text))])
        | ClientRegister(linkId) => object_([("linkId", string(linkId))])
        }
      )
    ])
  );
