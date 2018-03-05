open BsAbstract;

type linkId = string;

type userName = string;

type t =
  | Ping
  | MessageSend(linkId, userName, string)
  | MessageReceive(userName, string)
  | ClientRegister(linkId);

let get = Js.Dict.get |> Function.flip;

/**
 * Retrieve the textual key for the action
 */
let key = (action: t) =>
  switch action {
  | Ping => "PING"
  | MessageSend(_linkId, _userName, _text) => "MESSAGE_SEND"
  | MessageReceive(_userName, _text) => "MESSAGE_RECEIVE"
  | ClientRegister(_linkId) => "CLIENT_REGISTER"
  };

module Encode = {
  open Json.Encode;
  let payload = (action) =>
    switch action {
    | Ping => Js.Json.null
    | MessageSend(linkId, userName, text) =>
      object_([("linkId", string(linkId)), ("userName", string(userName)), ("text", string(text))])
    | MessageReceive(userName, text) =>
      object_([("userName", string(userName)), ("text", string(text))])
    | ClientRegister(linkId) => object_([("linkId", string(linkId))])
    };
  let action = (action: t) : Js.Json.t =>
    object_([("key", action |> key |> string), ("payload", action |> payload)]);
};

module Decode = {
  open Js.Json;
  open Option.Infix;
  let getWithDefault = Js.Option.getWithDefault;
  let payload = (payload, key) =>
    switch key {
    | "MESSAGE_SEND" =>
      Some(((linkId, userName, text) => MessageSend(linkId, userName, text)))
      <*> (payload |> get("linkId") >>= decodeString)
      <*> (payload |> get("userName") >>= decodeString)
      <*> (payload |> get("text") >>= decodeString)
    | "MESSAGE_RECEIVE" =>
      Some(((userName, text) => MessageReceive(userName, text)))
      <*> (payload |> get("userName") >>= decodeString)
      <*> (payload |> get("text") >>= decodeString)
    | "CLIENT_REGISTER" =>
      Some(((linkId) => ClientRegister(linkId))) <*> (payload |> get("linkId") >>= decodeString)
    | "PING" => Some(Ping)
    | _ => None
    };
  let action = (json: Js.Json.t) => {
    let action = json |> decodeObject;
    action
    >>= get("key")
    >>= decodeString
    >>= payload(action >>= get("payload") >>= decodeObject |> getWithDefault(Js.Dict.empty()))
  };
};
