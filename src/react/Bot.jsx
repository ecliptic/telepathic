import React from 'react'
import PropTypes from 'prop-types'
import castArray from 'lodash.castarray'

export const responseTypes = {
  YES_OR_NO: 'YES_OR_NO',
  SIMPLE: 'SIMPLE',
}

export const botPropTypes = {
  displayName: PropTypes.string,
  initialState: PropTypes.any,
  responses: PropTypes.arrayOf(PropTypes.shape({
    condition: PropTypes.any.isRequired,
    type: PropTypes.oneOf([
      responseTypes.YES_OR_NO,
      responseTypes.SIMPLE,
    ]).isRequired,
    text: PropTypes.oneOfType([
      PropTypes.string.isRequired,
      PropTypes.shape({
        YES: PropTypes.string,
        NO: PropTypes.string,
      }).isRequired,
    ]).isRequired,
    setState: PropTypes.oneOfType([
      PropTypes.object.isRequired,
      PropTypes.shape({
        YES: PropTypes.object,
        NO: PropTypes.object,
      }).isRequired,
    ]),
    isPrivate: PropTypes.oneOfType([
      PropTypes.bool.isRequired,
      PropTypes.shape({
        user: PropTypes.bool.isRequired,
        bot: PropTypes.bool.isRequired,
      }).isRequired,
    ]),
    newBotState: PropTypes.oneOfType([
      PropTypes.any.isRequired,
      PropTypes.shape({
        YES: PropTypes.any,
        NO: PropTypes.any,
      }),
    ]),
    sideEffect: PropTypes.oneOfType([
      PropTypes.func.isRequired,
      PropTypes.shape({
        YES: PropTypes.any,
        NO: PropTypes.any,
      }),
    ]),
  })).isRequired,
  disableOnUser: PropTypes.oneOfType([
    PropTypes.string.isRequired,
    PropTypes.bool.isRequired,
    PropTypes.arrayOf(PropTypes.string).isRequired,
  ]),
  render: PropTypes.func.isRequired,
}

class Bot extends React.Component {
  static propTypes = botPropTypes

  state = this.props.initialState || 1

  botMessage = newBotMessage => ({
    userName: this.props.displayName,
    text: newBotMessage,
  })

  handleBotResponse = (newMessage, messagesArray) => {
    const {responses, disableOnUser} = this.props

    // If the bot has not yet been disabled, determine its response to the new message
    if (this.state) {
      /* DISABLE ON USER */
      // Check to see if the bot should be disabled when certain users respond
      if (disableOnUser) {
        // If the bot should be disabled if any user starts replying in the chat...
        if (typeof disableOnUser === 'boolean') {
          // End the function call and set state to "null"
          return this.setState(null)
        }
        // Otherwise, cast disableOnUser as an array (even if it is one, already)
        const disableUserArray = castArray(disableOnUser)
        // And look through the current messages to see if a flagged user has responded
        if (messagesArray.find(message => disableUserArray.find(user => user === message.userName))) {
          // If so, end the function call and set state to "null"
          return this.setState(null)
        }
      }

      /* DETERMINE BOT RESPONSE */
      // Check to see if the current bot state matches a response condition
      // If it does not, do nothing
      const responseMatch = responses.find(response => this.state === response.condition)
      if (responseMatch) {
        // Perform different operations based on the response type
        switch (responseMatch.type) {
          /* YES OR NO RESPONSE */
          case responseTypes.YES_OR_NO:
            const lowerCaseMessage = newMessage.toLowerCase()
            // YES
            if (lowerCaseMessage.includes('yes') || lowerCaseMessage === 'y') {
              // Send the response message
              messagesArray.push(this.botMessage(responseMatch.text.YES || responseMatch.text))
              // Update the bot's state if a new one was provided
              const newBotState = responseMatch.newBotState.YES || responseMatch.newBotState
              if (newBotState) {
                this.setState(newBotState)
              }
              // Run the side effect, if one was provided
              const sideEffect = responseMatch.sideEffect.YES || responseMatch.sideEffect
              if (sideEffect) {
                sideEffect()
              }
            }
            // NO
            if (lowerCaseMessage.includes('no') || lowerCaseMessage === 'n') {
              // Send the response message
              messagesArray.push(this.botMessage(responseMatch.text.NO || responseMatch.text))
              // Update the bot's state if a new one was provided
              const newBotState = responseMatch.newBotState.NO || responseMatch.newBotState
              if (newBotState) {
                this.setState(newBotState)
              }
              // Run the side effect, if one was provided
              const sideEffect = responseMatch.sideEffect.NO || responseMatch.sideEffect
              if (sideEffect) {
                sideEffect()
              }
            }
            break

          /* SIMPLE RESPONSE */
          default:
            // Send the response message
            messagesArray.push(this.botMessage(responseMatch.text))
            const {newBotState, sideEffect} = responseMatch
            // Update the bot's state if a new one was provided
            if (newBotState) {
              this.setState(newBotState)
            }
            // Run the side effect, if one was provided
            if (sideEffect) {
              sideEffect()
            }
            break
        }
      }
    }
  }

  render () {
    return this.props.render({
      handleBotResponse: this.handleBotResponse,
    })
  }
}

export default Bot
