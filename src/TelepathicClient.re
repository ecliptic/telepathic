open WebSockets;

type t = {
  ws: WebSocket.t,
  linkId: string
};

type clientMessage = {. "userName": string, "text": string};

[@bs.module]
external generate : Js.t('a) => {. "spaced": string, "dashed": string, "raw": array(string)} =
  "project-name-generator";

[@bs.module] external titleCase : string => string = "title-case";

module Config = {
  type keys = {userName: string};
  let keys: keys = {userName: "telepathic:userName"};
};

/**
 * Choose a random name for a guest user
 */
let chooseName = () => titleCase(generate({"alliterative": true})##spaced);

/**
 * Get the current userName from localStorage
 */
let getName = () => Dom.Storage.(localStorage |> getItem(Config.keys.userName));

/**
 * Change the current userName in localStorage
 */
let updateName = (~name: string) =>
  Dom.Storage.(localStorage |> setItem(Config.keys.userName, name));

/**
 * A convenience wrapper around `chooseName` that updates the userName in localStorage
 */
let makeName = () => {
  let name = chooseName();
  updateName(~name);
  name
};

/**
 * Retrieve the userName from localStorage if one is present, and make a userName if not
 */
let getOrCreateUserName = () => {
  let name = getName();
  switch name {
  | Some(name) => name
  | _ => makeName()
  }
};

/**
 * Send a message to the Slack channel
 * */
let sendMessage = (~linkId: string, ~text: string, client: t) => {
  open TelepathicActions;
  open Js.Json;
  let userName = getOrCreateUserName();
  let message = MessageSend(linkId, userName, text);
  client.ws |> WebSocket.sendString(message |> Encode.action |> stringify)
};

let receiveMessage = (~onMessage, event) => {
  open TelepathicActions;
  let json = event |> WebSockets.MessageEvent.stringData |> Js.Json.parseExn;
  switch (json |> Decode.action) {
  | Some(MessageReceive(userName, text)) => onMessage({"userName": userName, "text": text})
  /* Otherwise, ignore */
  | _ => ()
  }
};

/**
 * Register the linkId with the server so that we can receive messages from the associated channel
 */
let register = (client: t) => {
  open TelepathicActions;
  open Js.Json;
  let message = ClientRegister(client.linkId);
  client.ws |> WebSocket.sendString(message |> Encode.action |> stringify)
};

/**
 * Initialize a new Client, optionally accepting an alternate socket client for testing
 * */
let make = (~socket=None, ~linkId: string, ~onMessage: clientMessage => unit, url: string) : t => {
  let ws =
    switch socket {
    | Some(socket) => socket
    | None => WebSocket.make(url)
    };
  let client = {ws, linkId};
  /* When connected, register the linkId with the server */
  ws
  |> WebSocket.on @@
  Open(() => register(client))
  |> WebSocket.on @@
  Message(receiveMessage(~onMessage))
  |> WebSocket.on @@
  Error((error) => Js.log("WebSocket error: " ++ error))
  |> WebSocket.on @@
  Close((event) => Js.log("WebSocket closed: " ++ WebSockets.CloseEvent.reason(event)))
  |> ignore;
  /* Return the client */
  client
};
