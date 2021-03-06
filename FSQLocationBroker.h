//
//  FSQLocationBroker.h
//
//  Copyright (c) 2014 foursquare. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreLocation/CoreLocation.h>

@protocol FSQLocationSubscriber, FSQRegionMonitoringSubscriber;

#pragma mark - FSQLocationBroker interface
/**
 Manager for location events application-wide. Subscribers must implement the
 _FSQXXXSubscriber_ protocol and themselves to the list of subscribers in order to receive
 notifications from the Broker.
 */

@interface FSQLocationBroker : NSObject <CLLocationManagerDelegate>
/**
 The most recent location received from the broker's CLLocationManager.
 
 @see [CLLocationManager location]
 */
@property (nonatomic, readonly) CLLocation *currentLocation;

/**
 The most accuracy we are currently requesting from the broker's CLLocationManager.
 
 @see [CLLocationManager desiredAccuracy]
 */
@property (nonatomic, readonly) CLLocationAccuracy currentAccuracy;

/** 
 The current set of location subscribers.
 
 Additions and removals of subscribers are processed on a background queue for thread safety reasons, so this
 set might not immediately reflect changes you make.
 */
@property (atomic, readonly) NSSet *locationSubscribers;

/** 
 The current set of region monitoring subscribers.
 
 Additions and removals of subscribers are processed on a background queue for thread safety reasons, so this
 set might not immediately reflect changes you make.
 */
@property (atomic, readonly) NSSet *regionSubscribers;

/**
 Access point to the location broker shared pointer/singleton. 
 
 Lazily instantiated the first time the method is called.
 
 Normally creates and returns an instance of FSQLocationBroker. If you would like to use a custom broker subclass
 in your app, you can change the class by using the @c setSharedClass: class method, but you must do this before
 the first call to @c shared.
 
 @return The shared singleton location broker instance.
 */
+ (instancetype)shared;

/**
 If you would like to use a custom subclass of FSQLocationBroker in your app, you can set its class using this
 method. All future calls to [FSQLocationBroker shared] will return the shared method of that class instead.
 
 If you specify a class that is not a subclass of FSQLocationBroker this method will do nothing.
 
 @warning If you want to use this method to change the broker class, you must call it before 
 the first call to @c shared.
 
 @param locationBrokerSubclass A subclass of FSQLocationBroker to use instead of the base implementation.
 */
+ (void)setSharedClass:(Class)locationBrokerSubclass;

/**
 Convenience method for getting if we are currently authorized to get location services from the CLLocationManager.
 
 @see [CLLocationManager authorizationStatus]
 
 @return YES if the app is authorized for location services, NO if not.
 */
+ (BOOL)isAuthorized;

/**
 Add a new location subscriber to the broker.
 
 The subscriber's location options will be taken into account when requesting locations services from the system.
 
 @param locationSubscriber Location subscriber to add. If this object is already in the broker's location subscriber
 list this method does nothing. The subscriber will be retained by the broker.

 @note Additions are processed on a background queue for thread safety reasons, and so might not be immediately
 reflected if you access the locationSubscribers property.
 */
- (void)addLocationSubscriber:(NSObject<FSQLocationSubscriber> *)locationSubscriber NS_REQUIRES_SUPER;

/**
 Remove a location subscriber from the broker.
 
 @param locationSubscriber Location subscriber to remove. If this object is not currently in the broker's location
 subscriber list this method does nothing.
 
 @note Removals are processed on a background queue for thread safety reasons, and so might not be immediately
 reflected if you access the locationSubscribers property.
 */
- (void)removeLocationSubscriber:(NSObject<FSQLocationSubscriber> *)locationSubscriber NS_REQUIRES_SUPER;

/**
 Updates the location services being requested from the system by the broker by checking the current list of
 location subscribers.
 
 This method is called automatically for you when a subscriber is added or removed, or when the relevant properties
 on the subscribers change if they are KVO-compliant.
 */
- (void)refreshLocationSubscribers NS_REQUIRES_SUPER;

/**
 Add a new region monitoring subscriber to the broker.
 
 @param regionSubscriber Region monitoring subscriber to add. If this object is already in the broker's region 
 monitoring subscriber list this method does nothing. The subscriber will be retained by the broker.

 @note Additions are processed on a background queue for thread safety reasons, and so might not be immediately
 reflected if you access the regionSubscribers property.
 */
- (void)addRegionMonitoringSubscriber:(NSObject<FSQRegionMonitoringSubscriber> *)regionSubscriber NS_REQUIRES_SUPER;

/**
 Remove a region monitoring subscriber from the broker.
 
 @param regionSubscriber Region monitoring subscriber to remove. If this object is not currently in the broker's 
 region monitoring subscriber list this method does nothing.
 
 @note Removals are processed on a background queue for thread safety reasons, and so might not be immediately
 reflected if you access the regionSubscribers property.
 */
- (void)removeRegionMonitoringSubscriber:(NSObject<FSQRegionMonitoringSubscriber> *)regionSubscriber NS_REQUIRES_SUPER;

/**
 Updates the location services being requested from the system by the broker by checking the current list of
 region monitoring subscribers.
 
 This method is called automatically for you when a subscriber is added or removed, or when the relevant properties
 on the subscribers change if they are KVO-compliant.
 */
- (void)refreshRegionMonitoringSubscribers NS_REQUIRES_SUPER;


/**
 Remove all subscribers of all types and turn off all location services.
 
 This can be useful if you want to completely reset your location services in some instance, 
 e.g. when a user logs out of your application.
 
 @note Removals are processed on a background queue for thread safety reasons, and so might not be immediately
 reflected if you access the locationSubscribers property.
 */
- (void)removeAllSubscribers NS_REQUIRES_SUPER;

@end

#pragma mark - FSQLocationSubscriber Protocol

/**
 Bitmask configuration options for the FSQLocationSubscriber protocol
 
 Subscribers should bitwise OR the options they want together.
 */
typedef NS_OPTIONS(NSUInteger, FSQLocationSubscriberOptions) {
    /**
     The subscriber wants the broker to subscribe for continuous location updates from the system.
     (i.e. [CLLocationManager startUpdatingLocation]).
     
     The broker will deliver these locations to the subscriber via the locationManagerDidUpdateLocations: method.
     
     If this option is present, the broker will use the subscriber's desiredAccuracy property to calculate
     what desired accuracy to report to the system.
     
     @note Including this option will make your subscriber receieve all location updates received by the broker
     from the system, regardless of which subscriber "caused" those updates.
     */
    FSQLocationSubscriberShouldRequestContinuousLocation    = (1 << 0),
    
    /**
     The subscriber wants the broker to subscribe for significant location updates from the system
     (i.e. [CLLocationManager startMonitoringSignificantLocationChanges]).
     
     The broker will deliver these locations to the subscriber via the locationManagerDidUpdateLocations: method.
     
     @note Including this option will make your subscriber receieve all location updates received by the broker
     from the system, regardless of which subscriber "caused" those updates.
     */
    FSQLocationSubscriberShouldMonitorSLCs                  = (1 << 1),
    
    /**
     The subscriber wants the broker to forward any location manager errors from the system to it.
     
     The broker will deliver these errors to the subscriber via the locationManagerFailedWithError: method.
     You must also implement this method on your subscriber to actually receive the errors.
     */
    FSQLocationSubscriberShouldReceiveErrors                = (1 << 2),
    
    /**
     The subscriber wants the broker to forward all locations received from the system to it.
     
     This is useful if you want to "passively" get location information, e.g. get any locations your app is already
     requesting because of other subscribers, but not actually affect what the broker requests from the sytem.
     
     This option does not have any effect if the ShouldRequestContinuousLocation or ShouldMonitorSLCs options
     are included (as those options effectively also imply this forwarding behavior).
     */
    FSQLocationSubscriberShouldReceiveAllBrokerLocations    = (1 << 3),
    
    /**
     The subscriber wants the broker to keep running its location requests when the app is backgrounded.
     
     When the app is backgrounded, the broker uses the subset of subscribers which include this option to relcalculate
     what location services to request from the system and which subscribers to deliver callbacks to.
     */
    FSQLocationSubscriberShouldRunInBackground              = (1 << 4),
};

/**
 The _FSQLocationSubscriber_ protocol.
 
 Each subscriber is responsible for specifying its desired settings (think of them as a data
 source). 
 
 The following properties **must be** KVO compliant OR **implementors must call** 
 [[FSLocationBroker shared] refreshLocationSubscribers] after changing their the return values 
 in order for changes to take place:
 
 * desiredAccuracy
 * subscriberOptions
 
 There is no guarantee changing the return values will affect _FSQLocationBroker_ behavior if
 you do not refresh the subscribers list. The broker will automatically try to observe and refresh after
 values change for KVO compliant properties.
 */
@protocol FSQLocationSubscriber<NSObject>

/**
 A bitmask of the configuration options for this subscriber. The broker will use this bitmask to determine
 what location services to request from the system.
 
 See the documentation for FSQLocationSubscriberOptions to get information on the available options.
 
 If the property is KVO-compliant, the broker will automatically update its state when changes occur. Otherwise
 you must manually call @c refreshLocationSubscribers on the broker to have your changes reflected.
 
 @note You can set this property to 0 to effectively remove this subscriber from the broker's consideration without
 actually removing it from the broker's subscriber list.
 */
@property (nonatomic, readonly) FSQLocationSubscriberOptions locationSubscriberOptions;

/**
 If the subscriber options include @c FSQLocationSubscriberShouldRequestContinuousLocation then this accuracy
 is used to calculate the desiredAccuracy to request from the system. Otherwise the value is unused.
 
 If the property is KVO-compliant, the broker will automatically update its state when changes occur. Otherwise
 you must manually call @c refreshLocationSubscribers on the broker to have your changes reflected.
 */
@property (nonatomic, readonly) CLLocationAccuracy desiredAccuracy;

/**
 Significant location change and continuous location update callbacks from the system will be forwarded to this method
 
 All SLC and continuous location updates received by the broker will be forwarded to all subscribers that requested
 either. E.g. a subscriber that only requested SLCs may receive continuous updates, or a subscriber which only 
 requested 3km accuracy will high accuracy updates if another subscriber requested 10m updates.

 @param locations The locations that were recieved.
 
 @see [CLLocationManagerDelegate locationManager:didUpdateLocations:]

 */
- (void)locationManagerDidUpdateLocations:(NSArray *)locations;

@optional

/**
 System location manager errors will be forwarded to this method.
 
 If your subscriber options property includes the FSQLocationSubscriberShouldReceiveErrors you must implement this
 method to actually receive the errors.
 
 @param error The error that was received.
 
 @see [CLLocationManagerDelegate locationManager:didFailWithError:]
 */
- (void)locationManagerFailedWithError:(NSError *)error;

@end

#pragma mark - FSQRegionMonitoringSubscriber Protocol

/**
 The protocol for subscribing to Region Monitoring events.
 
 The following properties **must be** KVO compliant OR **implementors must call**
 [[FSQLocationBroker shared] refreshRegionMonitoringSubscribers] after changing their the return values
 in order for changes to take place:
 
 * monitoredRegions
 
 There is no guarantee changing the return values will affect _FSQLocationBroker_ behavior if
 you do not refresh the subscribers list. The broker will automatically try to observe and refresh after
 values change for KVO compliant properties.
 */
@protocol FSQRegionMonitoringSubscriber<NSObject>

/**
 List of regions the subscriber wants to monitor.
 
 The FSQRegionMonitoringSubscriber must maintain its own list of monitored regions. 
 
 If the property is KVO-compliant, the broker will automatically update its state when changes occur. Otherwise
 you must manually call @c refreshLocationSubscribers on the broker to have your changes reflected.
 */
@property (nonatomic, readonly) NSSet *monitoredRegions;

/**
 If YES, the broker will forward location manager errors related to this subscriber's
 monitored region to the subscriber.
 
 The broker will deliver these errors to the subscriber via the locationManagerFailedWithError: method.
 You must also implement this method on your subscriber to actually receive the errors.
 */
@property (nonatomic, readonly) BOOL shouldReceiveRegionMonitoringErrors;

/**
 A subscriber must prefix all CLRegion.identifier with their prefix when monitoring a region.
 Format:
 
 [NSString stringWithFormat: "%@+%@", [self subscriberIdentifier], regionIdentifier]
 
 Valid identifiers have the same specification as valid C identifiers. The broker will
 assert on any regions whose identifiers are not specified with this format.
 
 Valid C identifiers: http://eli-project.sourceforge.net/c_html/c.html#s6.1.2
 */
- (NSString *)subscriberIdentifier;

/**
 This method should add the specified region to its list of monitored regions.
 
 The broker will use this method when reassigning the system's monitored regions to the matching subscribers
 (based on subscriber identifier) if they get out of sync (e.g. after an app restart).
 
 @param region A CLRegion to add to the subscriber's list of monitored regions.
 */
- (void)addMonitoredRegion:(CLRegion *)region;

/**
 Location manager didEnterRegion: calls for your monitored regions will be forwarded to this method.
 
 @see [CLLocationManagerDelegate locationManager:didEnterRegion:]
 
 @param region The region that was entered.
 */
- (void)didEnterRegion:(CLRegion *)region;

/**
 Location manager didExitRegion: calls for your monitored regions will be forwarded to this method.
 
 @see [CLLocationManagerDelegate locationManager:didExitRegion:]
 
 @param region The region that was exited.
 */
- (void)didExitRegion:(CLRegion *)region;

@optional

/**
 System location manager errors for your monitored regions will be forwarded to this method
 
 @param region The region for which an error occured.
 @param error  The error that was received.
 */
- (void)monitoringDidFailForRegion:(CLRegion *)region withError:(NSError *)error;

@end
