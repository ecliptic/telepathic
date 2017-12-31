open WebSockets;

type t = {
  ws: WebSocket.t,
  linkId: string
};

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
  client.ws |> WebSocket.sendString(message |> encode |> stringify)
};

/**
 * Register the linkId with the server so that we can receive messages from the associated channel
 */
let register = (client: t) => {
  open TelepathicActions;
  open Js.Json;
  let message = ClientRegister(client.linkId);
  client.ws |> WebSocket.sendString(message |> encode |> stringify)
};

/**
 * Initialize a new Client, optionally accepting an alternate socket client for testing
 * */
let make = (~socket=None, ~linkId: string, url: string) : t => {
  ws:
    switch socket {
    | Some(socket) => socket
    | None => WebSocket.make(url)
    },
  linkId
};

module ClientJs = {
  /** An interface for usage from plain JS */
  type t = {
    .
    "makeName": [@bs] (unit => string),
    "getName": [@bs] (unit => Js.Nullable.t(string)),
    "updateName": [@bs] (string => unit),
    "getOrCreateUserName": [@bs] (unit => string),
    "sendMessage": [@bs] (string => unit)
  };
  /**
   * Make a client and return the plain JS interface
   */
  let make: [@bs] ({. "linkId": string, "url": string} => t) =
    [@bs]
    (
      (options) => {
        let client = make(~linkId=options##linkId, options##url);
        {
          "makeName": [@bs] (() => makeName()),
          "getName": [@bs] (() => getName() |> Js.Nullable.from_opt),
          "updateName": [@bs] ((name) => updateName(~name)),
          "getOrCreateUserName": [@bs] (() => getOrCreateUserName()),
          "sendMessage": [@bs] ((text) => client |> sendMessage(~linkId=client.linkId, ~text))
        }
      }
    );
};

/** Give JavaScript clients easy access to the plain JS interface */
let start = ClientJs.make;
